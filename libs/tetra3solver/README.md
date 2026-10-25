`tetra3_solve.py` loads tetra3's own bundled `default_database.npz`
(FOV ~10-30deg, magnitude <=7) when `Tetra3Solver::setDatabasePath()`
is left unset - fine for wide-field work, no extra step needed.

A custom catalog is only worth generating for a narrower/wider field
of view than the default covers. It needs the raw Hipparcos catalog
(`hip_main.dat`, not bundled with the `tetra3` pip package - see
tetra3's own installation docs for where to get it) placed next to
where `tetra3` is installed, then:

```python
from tetra3 import Tetra3
t3 = Tetra3()
t3.generate_database(max_fov=30, save_as="catalog.npz")
```

Point `Tetra3Solver::setDatabasePath()` at the resulting file.

Runtime dependencies for `tetra3_solve.py` itself: `python3`, `numpy`,
`astropy`, `tetra3` (pip). Not yet wired into packaging - see
`notes/NOTES-backend-tetra3-plate-solver-evaluation.md`.
