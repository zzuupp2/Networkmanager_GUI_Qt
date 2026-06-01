# Networkmanager GUI Qt

A Linux network management GUI built with Qt 6, QML, and KF6NetworkManagerQt (Qt bindings for NetworkManager's D-Bus API).

## Features

- **Device management** — view all network devices with real-time state, MAC address, driver info
- **Wi-Fi scanning & connection** — scan available access points, connect/disconnect with security type and signal strength display
- **Connection profiles** — create, edit, delete, activate, and deactivate saved network connections
- **Runtime monitoring** — live IP address, gateway, DNS, interface speed, SSID, and connectivity status
- **Auto-reconnect prevention** — temporarily disables device autoconnect during user-initiated disconnection
- **Device filtering** — three-tier filtering (primary / primary+secondary / all devices)
- **System tray status** — top bar shows primary connection info, IP, and network connectivity indicator

## Requirements

- Qt 6.2+
- KF6NetworkManagerQt
- CMake 3.16+
- NetworkManager (running on the system)

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

```bash
./build/appNetworkmanager_GUI_Qt
```

## Directory Structure

```
.
├── CMakeLists.txt                  # Build configuration
├── main.cpp                        # Application entry, QML type registration
├── architecture.md                 # Detailed architecture documentation
├── qml/
│   ├── Main.qml                    # Root window with TopBar, SideBar, StackLayout
│   ├── components/
│   │   ├── TopBar.qml              # Primary connection info, network status, toggles
│   │   ├── SideBar.qml             # Three-page navigation (Devices / Wi-Fi / Saved Networks)
│   │   ├── DeviceCard.qml          # Device summary card with state color coding
│   │   ├── WifiItem.qml            # Wi-Fi AP card (SSID, security, band, signal strength)
│   │   ├── ConnectionEditorPanel.qml  # Full connection editing form
│   │   └── NewConnectionDialog.qml    # Two-step new connection wizard
│   └── pages/
│       ├── DevicePage.qml          # Device list with filter
│       ├── WifiPage.qml            # Wi-Fi AP list with scan
│       └── ConnectionsPage.qml     # Saved connections list + runtime status / editor
└── src/
    ├── wrapper/
    │   └── nm_wrapper.h/.cpp       # Central hub: exposes models/services to QML as "nm"
    ├── model/
    │   ├── ap/
    │   │   └── ap_model.h/.cpp     # Wi-Fi AP list model
    │   ├── connection/
    │   │   ├── con_editor_model.h/.cpp   # Connection editing form model (QML type)
    │   │   ├── con_list_model.h/.cpp     # Saved connections list model
    │   │   ├── con_runtime_model.h/.cpp  # Runtime state model (IP, gateway, DNS, etc.)
    │   │   └── con_setting_info.h/.cpp   # NM settings ↔ flat data structure
    │   └── device/
    │       ├── device_info.h/.cpp   # Device data struct from NM Device::Ptr
    │       └── device_model.h/.cpp  # Device list model
    ├── service/
    │   ├── ap_service.h/.cpp       # Wi-Fi scanning and AP tracking
    │   ├── con_runtime_service.h/.cpp  # Active connection state tracking
    │   ├── connection_manager.h/.cpp   # Connection CRUD + activate/deactivate
    │   └── device_service.h/.cpp   # Device lifecycle monitoring
    ├── filter/
    │   └── device_filter_proxy_model.h/.cpp  # Three-tier device filter proxy
    └── utils/
        └── network_utils.h/.cpp    # Enum conversion, type checks, formatting utilities
```

## Architecture

```
QML UI (Main.qml, pages, components)
  │
  ▼
NetworkManagerWrapper (exposed as "nm" context property)
  │
  ├── DeviceService ←→ DeviceModel ←→ DeviceFilterProxyModel → QML
  ├── ApService ←→ ApModel → QML
  ├── ConnectionRuntimeService ←→ ConnectionRuntimeModel → QML
  ├── ConnectionListModel → QML
  ├── ConnectionManager (CRUD + activate/deactivate)
  └── ConnectionEditorModel (editing form, QML type)
```

All NetworkManager interactions go through KF6NetworkManagerQt's D-Bus API. Services listen to NM signals and forward changes to models, which QML views bind to via property bindings.

See [architecture.md](architecture.md) for full data flow diagrams, signal chains, and module details.
