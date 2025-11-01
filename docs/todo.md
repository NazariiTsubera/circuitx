## Core Circuit Data Model
- [ ] Implement `Circuit` node registry with stable IDs, name lookup, and node-zero ground convention.
- [ ] Add templated `add<T>` element insertion that forwards to a unified `std::variant` storage.
- [ ] Implement `finalize()` pass to assign contiguous `VSrc.k` indices and validate element endpoints.
- [ ] Provide introspection helpers: `numNodeUnknowns()`, `numVsrc()`, element iteration utilities.

## Element Stamps & Assembly
- [ ] Encode MNA stamp routines for resistors, current sources, voltage sources, and capacitors (Backward Euler companion).
- [ ] Ensure stamping handles parallel elements by accumulation without depending on insertion order.
- [ ] Guard against invalid parameters (zero/negative R or C, unconnected nodes, conflicting voltage sources).
- [ ] Inject reachability checks to catch floating nets before factorization.

## Solver & Time Integration
- [ ] Build constant system matrix `A` from topology and `dt`, factor once per topology or `dt` change.
- [ ] Implement RHS builder per timestep: current sources, capacitor history currents, voltage source constraints.
- [ ] Integrate Backward Euler loop updating capacitor `v_prev` after each solve.
- [ ] Provide API to run transient simulations over user-defined time spans and sample intervals.
- [ ] Add regression tests covering single RC charge/discharge and mixed source scenarios.

## Visualization (SFML + Optional ImGui)
- [ ] Separate layout logic from rendering; define `VisualNode` metadata (positions, radii, colors).
- [ ] Prebuild `sf::VertexArray` for nodes (triangles per circle) and elements (laned glyphs) on topology/layout change.
- [ ] Implement element glyph generation: wires, resistor zig-zags, capacitor plates, source symbols.
- [ ] Add per-frame overlays for selection/hover while keeping geometry static.
- [ ] Integrate ImGui inspector (optional) for node/element inspection and simulation controls.

## Layout & Interaction
- [ ] Provide initial auto-layout (grid/BFS or force-directed) plus manual drag handles.
- [ ] Persist layout with the circuit definition to avoid rebuild jitter between runs.
- [ ] Support pan/zoom via `sf::View`, keeping world-space geometry consistent.
- [ ] Implement simple hit-testing on nodes/elements for interaction tooling.

## Performance & Robustness
- [ ] Cache LU factors and invalidate on topology/value/dt changes only.
- [ ] Batch draw calls (one for nodes, one for elements) and profile frame timings.
- [ ] Add unit parsing utility (`1k`, `10u`, etc.) feeding into element parameter constructors.
- [ ] Emit warnings/errors for inconsistent parallel voltage sources or disconnected subcircuits.
