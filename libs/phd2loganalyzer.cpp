#include "phd2loganalyzer.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_spline.h>

namespace
{

struct Row
{
    double t = 0;
    double raraw = 0, decraw = 0;
    double raguide = 0;
};

// Online mean/variance/covariance accumulator - same incremental formula
// phdlogview's LFit uses, avoids a separate pass to compute the mean first.
struct LinFit
{
    double avx = 0, avy = 0, varx = 0, covxy = 0, vary = 0, n = 0;
    void add(double x, double y)
    {
        double k = n;
        n += 1.0;
        k /= n;
        double dx = x - avx;
        double dy = y - avy;
        varx += (k * dx * dx - varx) / n;
        covxy += (k * dx * dy - covxy) / n;
        vary += (k * dy * dy - vary) / n;
        avx += dx / n;
        avy += dy / n;
    }
    double slope() const { return n >= 2. ? covxy / varx : 0.; }
    double intercept() const { return avy - slope() * avx; }
    double theta() const { return n >= 2. ? std::atan2(covxy, varx) : 0.; }
};

// Full RA periodic-error spectrum. undoCorrections=true reconstructs the
// uncorrected position (undoes every applied guide pulse) so the worm's
// true periodic error shows up even while being guided - useful to read
// off a PEC curve. false FFTs the actual guided residual as-is, showing
// what the guide loop left behind - useful to judge how well it's
// compensating.
std::vector<PHD2LogAnalyzer::Peak> computeSpectrum(const std::vector<Row> &rows, bool undoCorrections)
{
    const size_t n = rows.size();
    std::vector<double> t(n), rapos(n);
    {
        double prevRaraw = 0, prevRaguide = 0, pos = 0;
        for (size_t i = 0; i < n; i++)
        {
            const double move = rows[i].raraw - prevRaraw - prevRaguide;
            pos += move;
            prevRaraw = rows[i].raraw;
            prevRaguide = undoCorrections ? rows[i].raguide : 0.0;
            t[i] = rows[i].t;
            rapos[i] = pos;
        }
    }

    // Detrend: remove the linear drift, keep only the periodic + noise part.
    LinFit fit;
    for (size_t i = 0; i < n; i++)
        fit.add(t[i], rapos[i]);
    std::vector<double> rac(n);
    for (size_t i = 0; i < n; i++)
        rac[i] = rapos[i] - (fit.intercept() + fit.slope() * t[i]);

    // Resample to a uniform time step (Akima spline) - the FFT needs evenly
    // spaced samples but real guide frames don't arrive at a fixed rate.
    const double dt = (t[n - 1] - t[0]) / (double)(n - 1);
    std::vector<double> data(n * 2, 0.0);
    {
        gsl_interp_accel *acc = gsl_interp_accel_alloc();
        gsl_spline *spline = gsl_spline_alloc(gsl_interp_akima, n);
        gsl_spline_init(spline, t.data(), rac.data(), n);

        const double k = M_PI * 2.0 / (double)(n - 1);
        double x = t[0];
        for (size_t i = 0; i < n; i++, x += dt)
        {
            if (x > t[n - 1]) x = t[n - 1];
            const double hamming = 0.54 - 0.46 * std::cos((double)i * k);
            data[i * 2] = hamming * gsl_spline_eval(spline, x, acc);
        }

        gsl_spline_free(spline);
        gsl_interp_accel_free(acc);
    }

    gsl_fft_complex_workspace *work = gsl_fft_complex_workspace_alloc(n);
    gsl_fft_complex_wavetable *wt = gsl_fft_complex_wavetable_alloc(n);
    gsl_fft_complex_forward(data.data(), 1, n, wt, work);
    gsl_fft_complex_wavetable_free(wt);
    gsl_fft_complex_workspace_free(work);

    const size_t nfft = n / 2 - 1;   // omit DC (f=0) and the Nyquist-adjacent tail
    std::vector<PHD2LogAnalyzer::Peak> spectrum;
    spectrum.reserve(nfft);
    const double scale = 4.0 / (double)n;
    for (size_t i = 0; i < nfft; i++)
    {
        const gsl_complex *z = (const gsl_complex *)(&data[(i + 1) * 2]);
        const double f = (double)(i + 1) / ((double)n * dt);
        spectrum.push_back({1.0 / f, gsl_complex_abs(*z) * scale});
    }
    return spectrum;
}

// A spectral line spans several adjacent bins - keep only local maxima, so
// reporting the "top N" doesn't just repeat the same peak several times.
std::vector<PHD2LogAnalyzer::Peak> localMaximaPeaks(const std::vector<PHD2LogAnalyzer::Peak> &spectrum, int maxPeaks)
{
    std::vector<PHD2LogAnalyzer::Peak> peaks;
    for (size_t i = 1; i + 1 < spectrum.size(); i++)
    {
        if (spectrum[i].amplitudePx > spectrum[i - 1].amplitudePx && spectrum[i].amplitudePx > spectrum[i + 1].amplitudePx)
            peaks.push_back(spectrum[i]);
    }
    std::sort(peaks.begin(), peaks.end(), [](const PHD2LogAnalyzer::Peak & a, const PHD2LogAnalyzer::Peak & b)
    {
        return a.amplitudePx > b.amplitudePx;
    });
    if ((int)peaks.size() > maxPeaks)
        peaks.resize(maxPeaks);
    return peaks;
}

bool parseRows(const QString &path, double &pixelScaleArcsecPerPx, double &declinationRad,
                std::vector<Row> &rows)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&f);
    bool sawDataHeader = false;

    static const QRegularExpression reScale("Pixel scale = ([\\-0-9.]+)");
    static const QRegularExpression reDec("Dec = (-?[0-9.]+) deg");

    while (!in.atEnd())
    {
        QString line = in.readLine();

        if (!sawDataHeader)
        {
            if (line.startsWith("Frame,Time,mount"))
            {
                sawDataHeader = true;
                continue;
            }
            auto m = reScale.match(line);
            if (m.hasMatch())
                pixelScaleArcsecPerPx = m.captured(1).toDouble();
            m = reDec.match(line);
            if (m.hasMatch())
                declinationRad = m.captured(1).toDouble() * M_PI / 180.0;
            continue;
        }

        const QStringList f18 = line.split(',');
        if (f18.size() < 18)
            break;   // footer ("Guiding Ends at ...") or a torn last line - stop here

        bool ok1 = false, ok2 = false, ok3 = false, ok4 = false;
        Row r;
        r.t = f18[1].toDouble(&ok1);
        r.raraw = f18[5].toDouble(&ok2);
        r.decraw = f18[6].toDouble(&ok3);
        r.raguide = f18[7].toDouble(&ok4);
        if (!ok1 || !ok2 || !ok3 || !ok4)
            break;

        rows.push_back(r);
    }

    return sawDataHeader;
}

} // namespace

PHD2LogAnalyzer::Result PHD2LogAnalyzer::Analyze(const QString &logFilePath, int maxPeaks)
{
    Result res;

    if (logFilePath.isEmpty())
    {
        res.error = "no guide log for this session yet";
        return res;
    }

    double pixelScale = 0, declinationRad = 0;
    std::vector<Row> rows;
    if (!parseRows(logFilePath, pixelScale, declinationRad, rows))
    {
        res.error = "could not read or parse " + logFilePath;
        return res;
    }

    // Need enough points for the FFT's interpolation domain to make sense
    // (mirrors phdlogview's own guard: N/2-1 >= 5).
    enum { MIN_ROWS = 12 };
    if (rows.size() < MIN_ROWS)
    {
        res.error = QString("only %1 frame(s) so far - need at least %2").arg(rows.size()).arg((int)MIN_ROWS);
        return res;
    }

    const size_t n = rows.size();
    res.frameCount = (int)n;
    res.pixelScaleArcsecPerPx = pixelScale;

    // ---- residual RMS / peak / elongation (CalcStats) ----
    {
        LinFit fit;
        double peakR = 0, peakD = 0;
        for (const auto &r : rows)
        {
            fit.add(r.raraw, r.decraw);
            if (std::fabs(r.raraw) > std::fabs(peakR)) peakR = r.raraw;
            if (std::fabs(r.decraw) > std::fabs(peakD)) peakD = r.decraw;
        }
        res.rmsRaPx = std::sqrt(fit.varx);
        res.rmsDecPx = std::sqrt(fit.vary);
        res.peakRaPx = peakR;
        res.peakDecPx = peakD;

        const double theta = fit.theta();
        const double ct = std::cos(theta), st = std::sin(theta);
        LinFit fitxy;
        for (const auto &r : rows)
        {
            const double dr = r.raraw - fit.avx;
            const double dd = r.decraw - fit.avy;
            fitxy.add(dr * ct + dd * st, dd * ct - dr * st);
        }
        double a = std::sqrt(fitxy.varx), b = std::sqrt(fitxy.vary);
        if (a < b) std::swap(a, b);
        res.elongation = (a + b) > 1e-6 ? (a - b) / (a + b) : 1.0;
    }

    // ---- RA/DEC drift (px/min) + polar alignment error estimate ----
    {
        // RaDrift: reconstructed (uncorrected) RA position at the endpoints,
        // minus the sum of corrections actually applied in between, over time.
        double sum = 0;
        for (const auto &r : rows)
            sum += r.raguide;
        const double dtAll = rows.back().t - rows.front().t;
        res.driftRaPxPerMin = dtAll > 0
                               ? (rows.back().raraw - rows.front().raraw - sum) / dtAll * 60.0
                               : 0.0;

        // DecDrift: accumulate the DEC offset only across frames that weren't
        // actively corrected, then fit a line through the accumulated series.
        LinFit fit;
        double yAccum = 0;
        double prevY = rows.front().decraw;
        fit.add(rows.front().t, yAccum);
        for (size_t i = 1; i < n; i++)
        {
            const double dy = rows[i].decraw - prevY;
            yAccum += dy;
            fit.add(rows[i].t, yAccum);
            prevY = rows[i].decraw;
        }
        res.driftDecPxPerMin = fit.slope() * 60.0;

        if (declinationRad != 0 && std::fabs(std::cos(declinationRad)) > 1e-6)
        {
            // Barrett's polar alignment error estimate from DEC drift.
            res.polarAlignErrArcmin = 3.8197 * std::fabs(res.driftDecPxPerMin) * pixelScale
                                       / std::cos(declinationRad);
        }
    }

    // ---- RA periodic error spectrum (worm), raw and corrected ----
    res.spectrumRaw = computeSpectrum(rows, true);
    res.spectrumCorrected = computeSpectrum(rows, false);
    res.topPeriods = localMaximaPeaks(res.spectrumRaw, maxPeaks);

    res.ok = true;
    return res;
}
