#include "startracker.h"

#include <algorithm>
#include <cmath>

namespace startracker
{

namespace
{

/// Keep the `maxStars` brightest stars (leaves the list untouched if smaller).
QVector<Star> keepBrightest(QVector<Star> s, int maxStars)
{
    if (maxStars > 0 && s.size() > maxStars)
    {
        std::partial_sort(s.begin(), s.begin() + maxStars, s.end(),
                          [](const Star & a, const Star & b)
        {
            return a.flux > b.flux;
        });
        s.resize(maxStars);
    }
    return s;
}

/// For a candidate translation t = (tx,ty) expressed as (reference - current),
/// assign each reference star to the nearest current star within `gate` after
/// applying t, and return how many reference stars got a match.
int assign(const QVector<Star> &ref, const QVector<Star> &cur,
           double tx, double ty, double gate, QVector<int> &refToCur)
{
    refToCur.fill(-1, ref.size());
    const double g2 = gate * gate;
    int n = 0;
    for (int i = 0; i < ref.size(); ++i)
    {
        const double px = ref[i].x - tx;   // expected current position
        const double py = ref[i].y - ty;
        double best = g2;
        int bestJ = -1;
        for (int j = 0; j < cur.size(); ++j)
        {
            const double dx = cur[j].x - px;
            const double dy = cur[j].y - py;
            const double d2 = dx * dx + dy * dy;
            if (d2 < best)
            {
                best = d2;
                bestJ = j;
            }
        }
        if (bestJ >= 0)
        {
            refToCur[i] = bestJ;
            ++n;
        }
    }
    return n;
}

struct Pair
{
    double rx, ry;   // reference star
    double cx, cy;   // matched current star
    double w;        // weight
};

bool weightedMean(const QVector<Pair> &v, double &tx, double &ty)
{
    double sw = 0, sx = 0, sy = 0;
    for (const Pair &q : v)
    {
        sw += q.w;
        sx += q.w * (q.rx - q.cx);
        sy += q.w * (q.ry - q.cy);
    }
    if (sw <= 0)
        return false;
    tx = sx / sw;
    ty = sy / sw;
    return true;
}

} // namespace

MatchResult match(const QVector<Star> &refIn, const QVector<Star> &curIn,
                  double hintDx, double hintDy, const Params &p)
{
    MatchResult r;

    const QVector<Star> ref = keepBrightest(refIn, p.maxStars);
    const QVector<Star> cur = keepBrightest(curIn, p.maxStars);
    r.nRef = ref.size();
    r.nCur = cur.size();
    if (ref.size() < p.minInliers || cur.size() < p.minInliers)
        return r;

    // --- stage 1: vote for the dominant translation ------------------------
    // Candidate translations: the caller's hint, plus every (ref - cur) pair.
    // The winner is the one the most reference stars agree with at coarseGate.
    double bestTx = 0, bestTy = 0;
    int bestScore = -1;
    QVector<int> scratch;

    if (hintDx != 0.0 || hintDy != 0.0)
    {
        bestScore = assign(ref, cur, hintDx, hintDy, p.coarseGate, scratch);
        bestTx = hintDx;
        bestTy = hintDy;
    }

    for (int i = 0; i < ref.size(); ++i)
    {
        for (int j = 0; j < cur.size(); ++j)
        {
            const double tx = ref[i].x - cur[j].x;
            const double ty = ref[i].y - cur[j].y;
            const int s = assign(ref, cur, tx, ty, p.coarseGate, scratch);
            if (s > bestScore)   // strict '>' keeps the hint on ties (temporal stability)
            {
                bestScore = s;
                bestTx = tx;
                bestTy = ty;
            }
        }
    }

    r.nInliers = bestScore;   // best vote so far, useful even if we bail out below
    if (bestScore < p.minInliers)
        return r;

    // --- stage 2: robust refit -------------------------------------------------
    // Build the pair list from the winning translation, then iterate:
    // weighted mean -> residual RMS -> drop pairs beyond max(fineGate, k*sigma).
    QVector<int> map;
    assign(ref, cur, bestTx, bestTy, p.coarseGate, map);

    QVector<Pair> pairs;
    pairs.reserve(ref.size());
    for (int i = 0; i < ref.size(); ++i)
    {
        const int j = map[i];
        if (j < 0)
            continue;
        const double w = (p.fluxWeighted && ref[i].flux > 0.0) ? ref[i].flux : 1.0;
        pairs.push_back({ref[i].x, ref[i].y, cur[j].x, cur[j].y, w});
    }

    double tx = 0, ty = 0;
    if (pairs.size() < p.minInliers || !weightedMean(pairs, tx, ty))
        return r;

    double rms = 0;
    for (int pass = 0; pass <= p.sigmaClipIter; ++pass)
    {
        double s2 = 0, sw = 0;
        for (const Pair &q : pairs)
        {
            const double ex = (q.rx - q.cx) - tx;
            const double ey = (q.ry - q.cy) - ty;
            s2 += q.w * (ex * ex + ey * ey);
            sw += q.w;
        }
        rms = (sw > 0) ? std::sqrt(s2 / sw) : 0;

        if (pass == p.sigmaClipIter)
            break;   // final pass: keep the mean, no further culling

        const double lim = std::max(p.fineGate, p.sigmaClipK * rms);
        const double lim2 = lim * lim;
        QVector<Pair> kept;
        kept.reserve(pairs.size());
        for (const Pair &q : pairs)
        {
            const double ex = (q.rx - q.cx) - tx;
            const double ey = (q.ry - q.cy) - ty;
            if (ex * ex + ey * ey <= lim2)
                kept.push_back(q);
        }
        if (kept.size() == pairs.size())
            break;                       // converged
        if (kept.size() < p.minInliers)
            break;                       // stop before over-culling
        pairs = kept;
        weightedMean(pairs, tx, ty);
    }

    if (pairs.size() < p.minInliers)
        return r;

    r.ok = true;
    r.dx = tx;
    r.dy = ty;
    r.nInliers = pairs.size();
    r.rms = rms;
    return r;
}

} // namespace startracker
