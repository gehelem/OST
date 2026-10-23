#ifndef IMAGEQUALITY_h_
#define IMAGEQUALITY_h_

#include <cstdint>

// Generic, star-independent focus-quality metrics, shared by every module
// that wants an estimate of "how sharp is this frame" without relying on
// star detection (inspector, focus, ...). Each metric is relative to the
// scene it was computed on, not an absolute physical measure like a star's
// FWHM/HFR: only meaningful when comparing frames of the same target/
// exposure while sweeping focus position, not across sessions or targets.
//
// OpenCV stays an implementation detail of imagequality.cpp -- callers only
// ever see plain buffers/primitives, so linking this header in does not pull
// OpenCV into a module that doesn't already need it for something else.
namespace OST
{
// Laplacian variance: a standard, fast sharpness measure on textured frames
// (lunar/planetary surface detail, in-focus star fields, ...). The frame is
// stretched to 8 bits using [statMin, statMax] before the Laplacian is
// applied, matching the convention used elsewhere in OST for 8-bit contour
// work (cf. Inspector::analyzeFrame()).
//
// `roiPercent` (1-100, default 100 = full frame) restricts the analysis to a
// centered square-ish region whose width/height are each roiPercent% of the
// full frame's (e.g. 50 -> half width, half height, centered) - a caller
// drawing a matching overlay rectangle for the user should compute it the
// same way: w = width*roiPercent/100, h = height*roiPercent/100, centered.
//
// Assumes a mono buffer (bytesPerPixel 1 or 2); color/Bayer is not handled.
// Returns 0 on an invalid/empty buffer.
double laplacianVarianceSharpness(const uint8_t *buffer, int width, int height,
                                  int bytesPerPixel, double statMin, double statMax,
                                  double roiPercent = 100.0);
}

#endif
