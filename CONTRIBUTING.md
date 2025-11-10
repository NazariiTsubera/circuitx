# Contributing to CircuitX

Thanks for your interest in improving CircuitX! This project is structured around a clean separation of concerns (UI panels/services, circuit editor/simulator, solver handlers). The guidelines below help keep the codebase consistent and maintainable.

## Getting Started

1. **Fork & Clone**
   ```bash
   git clone https://github.com/<your-user>/circuitx.git
   cd circuitx
   ```
2. **Create a Branch**
   ```bash
   git checkout -b feature/my-awesome-change
   ```
3. **Build**
   ```bash
   cmake -S . -B build
   cmake --build build
   ```
   > First-time builds require network access because CMake FetchContent downloads SFML, ImGui-SFML, Eigen, and nlohmann/json.

## Code Style & Practices

- **C++26**: Use modern C++ features when they keep code clearer (structured bindings, `std::optional`, etc.).
- **Headers**: Guard every header with `#ifndef/#define`, keep includes minimal, and prefer forward declarations when possible.
- **Modules**: Favor small, focused classes (e.g., per-element solver handlers, per-panel UI classes). Don’t grow “God classes”.
- **UI Changes**:
  - Reuse `UiState` for persistent state.
  - Add new windows as separate panel classes under `app/ui/panels/`.
  - Avoid mixing rendering logic inside services unexpectedly.
- **Solver Changes**:
  - Add new element structs to `solver/include/circuitx/circuit.hpp`.
  - Implement stamping logic in its own handler under `solver/src/stamping/handlers/`.
  - Register the handler in `defaultStampRegistry()` (see `solver/src/stamping/StampContext.cpp`).

## Testing

- **Build**: Always run `cmake -S . -B build && cmake --build build` before opening a PR.
- **Runtime**: If your change touches UI/interaction, launch the app and verify the flow manually.
- **Solver**: When adding a new handler or modifying stamping logic, craft small circuits that exercise the new element (even if just printed logs) to ensure voltages/currents look correct.

## Documentation

- Update `README.md` if you add features visible to users.
- Keep `docs/architecture.md` current when changing high-level design (e.g., adding new services, solver steps).
- If you add non-trivial workflows, consider contributing a doc under `docs/`.

## Pull Requests

1. Rebase on `main` before submitting (`git pull --rebase origin main`).
2. Ensure the branch compiles and UI/solver flows affected by your change still work.
3. Describe the change clearly, referencing issues if applicable.
4. Expect review focus on modularity, readability, and adherence to existing patterns.

## Reporting Issues

Use GitHub Issues with:
- Clear description and reproduction steps.
- Logs or screenshots if UI-related.
- Extra context (OS, compiler, build type).

---

Thank you for helping keep CircuitX clean, modular, and pleasant to work on!
