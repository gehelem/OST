#!/usr/bin/env python3
"""Standalone plate-solve helper around esa/tetra3.

Invoked as a subprocess by OST's Tetra3Solver (libs/tetra3solver.cpp).
Solves one FITS image and prints a single JSON line to stdout. Exit
code 0 means solved, 1 means not solved or an error occurred.

Usage:
    tetra3_solve.py --image frame.fits --database catalog.npz
                     [--fov-estimate DEG] [--fov-max-error DEG]

The catalog database is generated once, offline - see README.md in
this directory, not part of this script.
"""
import argparse
import json
import sys

from astropy.io import fits
from tetra3 import Tetra3


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--image", required=True)
    parser.add_argument("--database", required=True)
    parser.add_argument("--fov-estimate", type=float, default=None)
    parser.add_argument("--fov-max-error", type=float, default=None)
    args = parser.parse_args()

    with fits.open(args.image) as hdul:
        data = hdul[0].data

    t3 = Tetra3(load_database=args.database)

    solve_kwargs = {}
    if args.fov_estimate is not None:
        solve_kwargs["fov_estimate"] = args.fov_estimate
    if args.fov_max_error is not None:
        solve_kwargs["fov_max_error"] = args.fov_max_error

    solution = t3.solve_from_image(data, **solve_kwargs)

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
