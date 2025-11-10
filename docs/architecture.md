# CircuitX Architecture

This document explains how the repository is organized, the responsibilities of each subsystem, and how they interact at runtime.

## High-Level Layers

1. **Application/UI (`app/`)**
   - Entry point (`main.cpp`, `Application`).
   - UI services and panels (ImGui-SFML).
   - Helpers (grid rendering, coordinate snapping, asset loading, wire tools).
2. **Solver (`solver/`)**
   - Public API (`solver/include/circuitx/circuit.hpp`).
   - Implementation (`solver/src/`) including stamping modules and per-element handlers.
3. **Assets (`res/`)**
   - Texture resources referenced by the UI palette/control panels.
4. **Docs (`docs/`)**
   - Architecture, TODOs, and other reference materials.

## Application Layer (`app/`)

### Entry & Composition

- `app/main.cpp` – instantiates `Application` and starts the main loop.
- `Application.{h,cpp}` – wires together the render window, services, controllers, and registers callbacks (e.g., resizing the canvas updates the `Visualizer`).

### Services and Controllers

- `services/CircuitController` – façade that ties the editor (`CircuitEditor`), simulator (`CircuitSimulator`), and grid helpers together. It handles commands (add/delete components, wires, simulations) while delegating actual work to the dedicated core services.
- `services/CircuitService` – in-memory storage for the circuit topology (nodes/elements) and entry point to the solver library.
- `services/CircuitView` – UI-friendly representation of components/nodes/wires used by the visualizer/UI when drawing overlays or hit-testing.
- `services/Visualizer` – draws the grid, components, wires, and overlays onto an SFML render target.
- `services/UiService` – orchestrates ImGui windows/panels using a shared `UiState`.

### Helpers & Panels

- `helpers/CoordinateTool.hpp` – grid snapping helpers.
- `helpers/GridRenderer.hpp` – draws the background grid.
- `helpers/AssetManager.hpp` – loads textures from `res/`.
- `helpers/WireTool.hpp` – manages multi-step wire placement interactions.
- `ui/` – shared UI state (`UiState.h`) and canvas drawing logic (`CanvasPanel`).
- `ui/panels/` – one class per ImGui window (Palette, Toolbox, Control Panel, Simulation, Settings, Properties, Topology). Each panel owns only the dependencies it needs (e.g., Simulation panel uses `CircuitController` for results, Toolbox panel uses `CircuitController` + `CoordinateTool` for edits).

## Solver Layer (`solver/`)

### Public Interface

- `solver/include/circuitx/circuit.hpp`
  - Defines `Node`, element variants (`Res`, `Cap`, `VSource`, `ISource`, `Wire`), `Circuit`, and `TransientResult`.
  - Consumers (e.g., `CircuitService`) interact with this header only.

### Implementation (`solver/src/`)

- `circuit.cpp`
  - Handles topology unification (merging wires into supernodes).
  - Builds Modified Nodal Analysis contexts and requests stamping from the registry.
  - Runs steady-state (`solve`) and transient (`simulateTransient`) analyses.
  - Serializes circuits to JSON for UI inspection.
- `stamping/`
  - `MnaContext.h` – node/voltage bookkeeping structure.
  - `StampContext.{h,cpp}` – shared stamping context, capacitor state helpers, registry.
  - `handlers/*.{h,cpp}` – individual element handlers:
    - `ResistorStampHandler`
    - `VoltageSourceStampHandler`
    - `CurrentSourceStampHandler`
    - `CapacitorStampHandler`
  - To add a new component:
    1. Define a new element type in `circuit.hpp`.
    2. Implement a handler (e.g., `InductorStampHandler`) that inherits `ElementStampHandler`.
    3. Register it in `defaultStampRegistry()` so it participates in stamping.

### Transient Simulation Flow

1. Assemble base conductance matrix/vector via handlers.
2. Detect capacitors and store them as `CapacitorState` instances.
3. Solve for steady state to seed capacitor voltages.
4. For each timestep:
   - Copy the base matrix/vector.
   - Add discretized capacitor conductances/currents.
   - Solve the linear system.
   - Record node voltages and update capacitor history terms.

## Documentation (`docs/`)

- `architecture.md` – this file.
- `todo.md` – backlog/ideas (existing file retained for project notes).

## Adding New Functionality

1. **UI Features** – create a new panel class in `app/ui/panels/`, update `UiService` to instantiate and call it, and extend `UiState` if persistent state is required.
2. **Solver Components** – add a new handler under `solver/src/stamping/handlers/` and register it in the default registry. Update `circuit.hpp` with the new element struct/variant entry.
3. **Assets** – drop new textures into `res/` and register them in `AssetManager`.

Stick to the “single responsibility” guidelines (small panels/services/handlers) to keep the codebase scalable as CircuitX grows.
