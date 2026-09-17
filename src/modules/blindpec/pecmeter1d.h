/**
 * @file pecmeter1d.h
 * @brief PROTOTYPE - 1D-projection sub-pixel translation estimator.
 *
 * BlindPEC only cares about the displacement along one known axis (theta, from
 * the free-run characterization) - the perpendicular component is diagnostic
 * only (see pecmeter.h). Instead of registering the full 2D frame, this
 * collapses each frame to a 1D intensity profile along theta (summing across
 * the perpendicular direction) before doing the sub-pixel correlation. Two
 * effects, both aimed at the measurement-noise-floor discussion (not the
 * control loop, which is a separate question):
 *
 *   1. Summing N rows of per-pixel noise reduces the effective noise on the
 *      profile roughly like the texture's cross-axis correlation length
 *      allows - "software binning" along the direction that carries no useful
 *      information for this driver, for free, no hardware change. How much it
 *      actually buys depends on the real surface texture (see the bench).
 *   2. The upsampled-DFT sub-pixel zoom (Guizar-Sicairos) costs O(kappa)
 *      matmul entries here instead of O(kappa^2) for the 2D case (pecmeter.cpp)
 *      - the CPU cost that shelved measParams/dftshift on the live module
 *      (too slow at -O0) mostly disappears.
 *
 * VERDICT (see blindpec-notes.fr.md, "Piste explorée et abandonnée"): this
 * idea does NOT work, on synthetic AND real BlindPEC frames both - summing
 * rows averages away real texture signal (short cross-axis correlation
 * length) as fast as it averages away noise, so the projected profile
 * correlates worse, not better, than the full 2D frame. Kept in the repo,
 * not wired into blindpec.cpp / the live pecmeter::Meter, as a documented
 * dead end so the idea is not retried blind. Qt/INDI-free like pecmeter,
 * OpenCV only, not part of the CMake build.
 */
#pragma once

#include <opencv2/opencv.hpp>

namespace pecmeter1d
{

/// Collapse `frame` (any type, converted to CV_32F) to a 1D profile along
/// direction `thetaRad` (image convention: 0 = +X, increasing toward +Y).
/// thetaRad == 0 is a plain column-sum (exact, no interpolation); any other
/// angle rotates the frame first (bilinear) then sums - the one place this
/// method spends accuracy for generality. Output: 1 x N CV_32F row.
cv::Mat project(const cv::Mat &frame, double thetaRad);

/// Sub-pixel shift (px, along the profile) of `cur` w.r.t. `ref` by
/// upsampled-DFT cross-correlation (Guizar-Sicairos, 1D). Both 1 x N CV_32F,
/// same size. `kappa` = 1/resolution of the local zoom (same meaning as
/// measParams/upsamplefactor on the 2D estimator).
double upsampledShift(const cv::Mat &ref, const cv::Mat &cur, double kappa);

} // namespace pecmeter1d
