/**
 * @file startracker.h
 * @brief Robust star-field drift measurement for the guider.
 *
 * Replaces the old triangle-index matcher (`buildIndexes` / `matchIndexes`).
 * Between two guider frames the star field undergoes a near-pure translation
 * (large during calibration, sub-pixel during guiding; a meridian flip's 180°
 * rotation is handled separately by the guider's reverse flags). So instead of
 * rotation/scale-invariant triangle fingerprints we look directly for the
 * dominant translation:
 *
 *   1. every (reference star, current star) pair implies a translation
 *      `t = cur - ref`; the true translation is the one the most pairs agree on
 *      (a small RANSAC / voting step, immune to flux-rank reshuffle and to
 *      stars appearing/disappearing);
 *   2. that translation's inliers are then refined with a two-stage gate and
 *      iterative sigma-clipping, and the final drift is a flux-weighted mean of
 *      the surviving residuals -> continuous, sub-pixel (~0.05-0.1 px with a
 *      dozen inliers).
 *
 * Deliberately free of any StellarSolver / Qt-widget dependency so it can be
 * unit-tested in isolation: feed two `QVector<Star>` and check `dx/dy`.
 */
#pragma once

#include <QVector>

namespace startracker
{

/// One detected star. `flux` is used for weighting and for keeping the
/// brightest N; pass 0 for all stars to fall back to unit weights.
struct Star
{
    double x = 0;
    double y = 0;
    double flux = 0;
};

struct Params
{
    int    maxStars      = 30;   ///< cap on stars used from each list (brightest kept)
    double coarseGate    = 4.0;  ///< px — voting search radius (stage 1)
    double fineGate      = 1.0;  ///< px — inlier re-selection radius after the first refit (stage 2)
    int    minInliers    = 3;    ///< below this the result is not trusted (ok = false)
    int    sigmaClipIter = 2;    ///< robust refit passes
    double sigmaClipK    = 2.5;  ///< reject residuals beyond k * sigma
    bool   fluxWeighted  = true; ///< weight the final mean by star flux
};

struct MatchResult
{
    bool   ok       = false; ///< true if a confident translation was found
    double dx       = 0;     ///< reference minus current, pixels (same sign convention as the old matchIndexes)
    double dy       = 0;
    int    nInliers = 0;     ///< stars that agreed with the best translation (filled even when ok == false)
    int    nRef     = 0;     ///< stars used from the reference list (after the maxStars cap)
    int    nCur     = 0;     ///< stars used from the current list
    double rms      = 0;     ///< RMS of the inlier residuals, px (drift-measurement quality)
};

/**
 * @brief Measure the translation between a reference star list and a current one.
 * @param refStars  reference frame stars (any order)
 * @param curStars  current frame stars (any order)
 * @param hintDx    predicted `ref - cur` translation from the previous frame,
 * @param hintDy    used to seed / stabilise the vote; pass 0,0 if unknown
 * @param p         tuning parameters
 * @return dx/dy = mean(reference - current) over inliers; `ok` false if the
 *         field could not be correlated (caller should skip the frame, not
 *         necessarily abort).
 */
MatchResult match(const QVector<Star> &refStars,
                  const QVector<Star> &curStars,
                  double hintDx = 0.0, double hintDy = 0.0,
                  const Params &p = Params());

} // namespace startracker
