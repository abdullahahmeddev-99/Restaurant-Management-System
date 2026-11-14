cmake_minimum_required(VERSION 3.16)
project(RestaurantGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CORE_SOURCES
    core/validation.cpp
    core/order_manager.cpp
    core/meal_logic.cpp
    core/sales_manager.cpp
)

set(GUI_SOURCES
    gui/main_window.cpp
    gui/menu_view.cpp
    gui/new_order_view.cpp
    gui/sales_view.cpp
    gui/dialogs/pizza_dialog.cpp
    gui/dialogs/burger_dialog.cpp
    gui/dialogs/potatoes_dialog.cpp
    gui/dialogs/chicken_dialog.cpp
    gui/dialogs/payment_dialog.cpp
    gui/dialogs/card_details_dialog.cpp
    gui/dialogs/otp_verification_dialog.cpp
    gui/dialogs/invoice_dialog.cpp
    gui/dialogs/quantity_spin_box.cpp
    gui_main.cpp
)

add_executable(RestaurantGUI
    ${CORE_SOURCES}
    ${GUI_SOURCES}
)

target_include_directories(RestaurantGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RestaurantGUI PRIVATE Qt6::Widgets)

add_executable(GuiCheckoutTest
    test_gui_checkout.cpp
    core/validation.cpp
    core/order_manager.cpp
    core/meal_logic.cpp
    core/sales_manager.cpp
    gui/dialogs/payment_dialog.cpp
    gui/dialogs/card_details_dialog.cpp
    gui/dialogs/otp_verification_dialog.cpp
    gui/dialogs/invoice_dialog.cpp
)
target_include_directories(GuiCheckoutTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(GuiCheckoutTest PRIVATE Qt6::Widgets)

add_executable(CoreTest
    test_core.cpp
    core/validation.cpp
    core/order_manager.cpp
    core/meal_logic.cpp
    core/sales_manager.cpp
)
target_include_directories(CoreTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(CoreInventoryTest
    test_core_inventory.cpp
    core/validation.cpp
    core/order_manager.cpp
    core/meal_logic.cpp
    core/sales_manager.cpp
)
target_include_directories(CoreInventoryTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
