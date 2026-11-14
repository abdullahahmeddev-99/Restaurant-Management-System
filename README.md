# Restaurant Management System

نظام إدارة مطعم بواجهة رسومية (Qt 6) مبني بلغة C++17، مع فصل كامل لمنطق العمل
(business logic) عن واجهة المستخدم. مبني على نسخة console أصلية تمت إعادة هيكلتها
إلى طبقة `core` مستقلة + طبقة `gui`.

A restaurant management desktop app built with **C++17** and **Qt 6 Widgets**. The
business rules live in a UI-independent `core` engine, and the Qt `gui` layer only
handles presentation.

## Features

- Interactive menu with meal cards (Pizza, Burger, Potatoes, Chicken).
- Order cart: add/remove items, change quantities, live totals.
- Configurable meals: pizza sizes and chicken portions.
- Offers & discounts:
  - Pizza: buy 2 get 1 free, extra 10% off for quantity >= 4.
  - Burgers: combo offer for quantity 2.
  - Potatoes: buy 2 get 1 free, 5% off for quantity >= 4.
  - Chicken: 5% off for quantity >= 2, free potatoes with a full chicken.
  - 5% discount card, 10% burger + potatoes combo, 14% tax, delivery fee.
- Checkout with Cash, Credit Card (card details + OTP simulation), and Wallet.
- Stock/inventory checks before payment and safe deduction after payment.
- Invoice preview and sales report dashboard.
- Unit tests for core logic, inventory, and the checkout GUI flow.

## Tech Stack

- C++17
- Qt 6 Widgets
- CMake (>= 3.16) + Ninja (recommended)
- GCC / MinGW / Clang / MSVC

## Requirements

- CMake 3.16+
- A C++17 compiler
- Qt 6 with the `Widgets` module (`Qt6::Widgets`)
- Ninja (recommended, optional)

## Build & Run

### Windows (MSYS2 / MinGW)

```powershell
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake `
    mingw-w64-x86_64-ninja mingw-w64-x86_64-qt6

cmake -S . -B build -G Ninja `
    -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 `
    -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Make the Qt runtime DLLs discoverable, then launch
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
.\build\RestaurantGUI.exe
```

### Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install -y qt6-base-dev cmake ninja-build g++

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/RestaurantGUI
```

### macOS (Homebrew)

```bash
brew install qt cmake ninja
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
./build/RestaurantGUI
```

> Tip: add `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` to the configure step to
> generate `compile_commands.json` for clangd / editor tooling.

## Tests

The project builds three test executables and registers them with CTest:

```bash
cmake --build build
cd build
ctest --output-on-failure
```

Or run them directly:

```text
CoreTest            # validation, offers, calculation, payment, cart flow
CoreInventoryTest   # stock checks, deduction, no-negative / no-double-deduction
GuiCheckoutTest     # Qt checkout flow: cash, card + OTP, wallet, invoice
```

`GuiCheckoutTest` runs headless through the `offscreen` Qt platform plugin
(configured automatically in CTest). To run it manually on a headless machine:

```bash
QT_QPA_PLATFORM=offscreen ./build/GuiCheckoutTest
```

## Project Structure

```text
core/                 Business logic (no Qt dependency)
  models.h            Data structures and default prices/stock
  validation.*        Name / phone validation
  order_manager.*     Cart, calculation, inventory, payment processing
  meal_logic.*        Meal-specific offers and pricing rules
  sales_manager.*     Sales totals and change calculation

gui/                  Qt 6 presentation layer
  main_window.*       Main window + navigation
  menu_view.*         Menu cards
  new_order_view.*    Cart and order editing
  sales_view.*        Sales dashboard
  dialogs/            Pizza, burger, potatoes, chicken, payment, card,
                      OTP, invoice dialogs

gui_main.cpp          Application entry point
test_*.cpp            Test executables
CMakeLists.txt        Build configuration
```

## Notes

- Prices, stock levels, tax rate, and delivery fee are defined in
  `core/models.h` (`RestaurantData`).
- `main.cpp` in the repository root is the original console prototype and is
  **not** part of the CMake build; the GUI entry point is `gui_main.cpp`.
