#!/usr/bin/env python3
"""Standalone plate-solve helper around esa/tetra3.

Invoked as a subprocess by OST's Tetra3Solver (libs/tetra3solver.cpp).
Solves one FITS image and prints a single JSON line to stdout. Exit
code 0 means solved, 1 means not solved or an error occurred.

Usage:
    tetra3_solve.py --image frame.fits [--database catalog.npz]
                     [--fov-estimate DEG] [--fov-max-error DEG]

Without --database, tetra3's own bundled default_database.npz is used
(FOV ~10-30deg, magnitude <=7 - fine for wide-field work). A custom
catalog is a separate, optional step - see README.md in this directory.
"""
import argparse
import json
import sys

from astropy.io import fits
from PIL import Image
from tetra3 import Tetra3


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--image", required=True)
    parser.add_argument("--database", default=None)
    parser.add_argument("--fov-estimate", type=float, default=None)
    parser.add_argument("--fov-max-error", type=float, default=None)
    args = parser.parse_args()

    with fits.open(args.image) as hdul:
        data = hdul[0].data

    # solve_from_image() wants a PIL.Image, not a raw numpy array - it
    # reads image.size (a PIL attribute), see tetra3.py's Tetra3.solve_from_image().
    image = Image.fromarray(data)

    t3 = Tetra3(load_database=args.database) if args.database else Tetra3()

    solve_kwargs = {}
    if args.fov_estimate is not None:
        solve_kwargs["fov_estimate"] = args.fov_estimate
    if args.fov_max_error is not None:
        solve_kwargs["fov_max_error"] = args.fov_max_error

    # binary_open (tetra3 default True) erases single-pixel detections via
    # morphological opening - real stars that light up only one pixel
    # (undersampled optics, short focal length) get discarded entirely,
    # leaving zero centroids. Confirmed on a real OST frame (135mm/5.2um,
    # ~7.9"/px): 94 plausible star pixels, all wiped out with the default.
    solution = t3.solve_from_image(image, binary_open=False, min_area=1, **solve_kwargs)

    ra = solution.get("RA")
    if ra is None:
        print(json.dumps({"solved": False}))
        return 1

    fov_deg = solution.get("FOV")
    height_px, width_px = data.shape[:2]
    pixscale_arcsec = (fov_deg * 3600.0 / width_px) if fov_deg else 0.0

    result = {
        "solved": True,
        "ra": ra,
        "dec": solution.get("Dec"),
        "orientation": solution.get("Roll"),
        "fieldWidthArcmin": (fov_deg * 60.0) if fov_deg else 0.0,
        "fieldHeightArcmin": (fov_deg * height_px / width_px * 60.0) if fov_deg else 0.0,
        "pixscaleArcsecPerPix": pixscale_arcsec,
    }
    print(json.dumps(result))
    return 0


if __name__ == "__main__":
    sys.exit(main())
