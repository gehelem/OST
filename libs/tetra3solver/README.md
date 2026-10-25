`tetra3_solve.py` needs a pre-generated star pattern catalog (a `.npz`
file) to load at each solve - this is a one-time, offline step, not
part of the per-solve script:

```python
from tetra3 import Tetra3
t3 = Tetra3()
t3.generate_database(max_fov=30, save_as="catalog.npz")
```

`max_fov` should cover the widest field of view any camera/optics
combination will actually solve with (see `fov_estimate`/
`fov_max_error` in `Tetra3Solver::SolveStars()`). Point
`Tetra3Solver::setDatabasePath()` at the resulting file.

Runtime dependencies for `tetra3_solve.py` itself: `python3`, `numpy`,
`astropy`, `tetra3` (pip). Not yet wired into packaging - see
`notes/NOTES-backend-tetra3-plate-solver-evaluation.md`.
