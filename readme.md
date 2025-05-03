## Cursor MDC Rules for Reflow Oven Project (Raspberry Pi Pico, FreeRTOS)

This document defines the architecture and structural conventions of the project. It enforces clean separation of concerns via MVC, FreeRTOS-based concurrency, and embedded hardware best practices.

---

### 📌 Project Foundation
- Microcontroller: **Raspberry Pi Pico**
- Toolchain: **C++ with the Raspberry Pi Pico C SDK**
- RTOS: **FreeRTOS (SMP-aware)**
- Architecture: **MVC + Services**
- Rendering: **lcdgfx (SSD1331)**

---

### 🧱 MVC Architectural Rules

#### Views
- Must inherit from `UIView` (in `ui_view.h`).
- Must not interact with **services**, **models**, or **hardware** directly.
- May only interact with their assigned **controller** via input events.
- May **render UI**, hold **local UI state**, and delegate control logic.
- UI logic should be composed using **reusable UI components** from `src/ui/components`.
- Views are **registered by controllers** using `UIViewService::registerView()`.

#### Controllers
- Must inherit from `BaseController`.
- Are responsible for linking views to services/models.
- May:
  - Call into `Service` or `Model` classes.
  - Push/pop views using `UIViewService`.
- Must not do long-running or real-time hardware work directly — offload to a service.

#### Services
- Must be singleton classes (typically accessed via `getInstance()`).
- Run background logic or hardware control using **FreeRTOS tasks**.
- Can use static task creation, semaphores, event groups, or queues.
- May expose methods for controllers to call (e.g., `startFan()`, `readTemperature()`).
- May not access UI or views.

#### Models
- Must inherit from `BaseModel`.
- Hold shared or persistent data.
- May be updated by services and queried by controllers.
- Must not reference views or services.

---

### 📁 File & Folder Structure

| Path                     | Contents                                                |
|--------------------------|----------------------------------------------------------|
| `src/controllers`        | Controller classes (`ReflowController`, `MainMenuController`) |
| `src/services`           | Hardware interfaces, background tasks (`FanService`, `SensorService`) |
| `src/models`             | App state and configuration (`ReflowModel`, `SettingsModel`) |
| `src/ui/views`           | View classes (`MainMenuView`, `CalibrationSummaryView`) |
| `src/ui/components`      | Reusable UI elements (`Menu`, `MenuItem`, `Graph`) |
| `src/types`              | Shared types, enums, structs |
| `src/constants.h`        | All GPIO numbers, I2C/SPI bus, hardware thresholds |
| `src/globals.h/cpp`      | Utility functions, shared helpers |
| `src/isr_handlers.h/cpp` | Centralized ISR handler registration & dispatch |
| `src/library`            | Custom reusable modules (non-third party) |
| `lib/`                   | External libraries (`lcdgfx`, `FreeRTOS`) |

---

### 📦 Reuse and Base Class Contracts

All core app classes must extend their respective base interfaces:

| Purpose     | Base Class         | Location                |
|-------------|--------------------|--------------------------|
| Controller  | `BaseController`   | `src/base_controller.h` |
| Model       | `BaseModel`        | `src/base_model.h`      |
| View        | `UIView`           | `src/ui/ui_view.h`      |

This ensures interface consistency, lifecycle control (`beforeLoad()`), and proper type integration with `UIViewService`.

---

### ⚙️ FreeRTOS Best Practices

- Prefer **FreeRTOS queues, semaphores, and tasks** for concurrency.
- Use `vTaskDelay()` or timers instead of `sleep` or polling loops.
- Allocate tasks statically where possible.
- Avoid `malloc`/`new` in ISRs or time-critical logic.

---

### 🧼 Naming Conventions

| Category           | Convention                |
|--------------------|----------------------------|
| Classes            | `PascalCase` (e.g. `FanService`, `SettingsModel`) |
| Methods / Variables| `camelCase` (e.g. `handleInput`, `startReflow()`) |
| Files              | `snake_case.cpp/h` and match class name |
| Constants          | `UPPER_CASE_SNAKE` (defined in `constants.h`) |
| Views              | `SomethingView` (e.g., `MainMenuView`) |
| Components         | `MenuComponent`, `GraphComponent` |
| Controllers        | `SomethingController` |
| Models             | `SomethingModel` |
| Services           | `SomethingService` |

---

### 📐 UI Componentization

- All reusable UI pieces must live in `src/ui/components`.
- Views must **compose components**, not build UI from scratch.
- Avoid inline layout logic in views; delegate to components when possible.

---

### 🔐 Enforcement Summary

| ✅ Allowed                        | ❌ Disallowed                        |
|----------------------------------|--------------------------------------|
| Views → Controllers only         | Views → Services, Models, Hardware   |
| Controllers → Models/Services    | Controllers → Hardware directly      |
| Services → Hardware only         | Services → Views/UI                  |
| FreeRTOS for concurrency         | Blocking logic or busy-wait loops    |
| Centralized ISR dispatch         | Random ISR attachments everywhere    |

---

Would you like me to generate a `DESIGN_GUIDELINES.md` file with this content to drop into your repo?
