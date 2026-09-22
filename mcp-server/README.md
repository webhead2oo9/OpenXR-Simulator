# OpenXR Simulator MCP Server

A Model Context Protocol (MCP) server that provides tools for diagnosing OpenXR issues, capturing screenshots, and driving the simulated headset in the OpenXR Simulator runtime.

## Features

- **Screenshot Capture**: Capture the current XR frame being rendered (stereo view)
- **Frame Diagnostics**: Get detailed frame timing, resolution, and format information
- **Log Analysis**: Read and analyze simulator logs for debugging
- **Issue Diagnosis**: Automated analysis of common OpenXR problems
- **Session Monitoring**: Track session state and head tracking
- **Flicker Detection**: Inspect whole-frame and UI quad-layer continuity, with contact sheets an agent can review visually
- **Headset Control**: Drive head pose, FOV, IPD and headset profiles to surface projection and handedness bugs
- **Stereo Validation**: Measure left/right disparity and preview stereo as a red/cyan anaglyph

## Installation

### Prerequisites
- Python 3.10 or higher
- The OpenXR Simulator runtime, either active machine-wide or selected per
  process with `XR_RUNTIME_JSON` (see [Using the simulator without activating it](#using-the-simulator-without-activating-it))

### Install from source
```bash
cd mcp-server
pip install -e .
```

Or install the dependencies directly (the MCP SDK is pinned below 2.0):
```bash
pip install -r requirements.txt
```

A virtual environment keeps these apart from other Python tools:
```bash
cd mcp-server
python -m venv .venv
.venv\Scripts\pip install -r requirements.txt
```

## Usage

### Running the MCP Server

```bash
python openxr_simulator_mcp.py
```

The server speaks MCP over stdio, so an MCP client normally starts it for you.

### Configuring with Claude Code

Register the server once for every project:

```bash
claude mcp add openxr-simulator --scope user -- python C:/path/to/OpenXR-Simulator/mcp-server/openxr_simulator_mcp.py
```

If you use a virtual environment, give its interpreter instead of `python`
(for example `C:/path/to/OpenXR-Simulator/mcp-server/.venv/Scripts/python.exe`).
Check it with `claude mcp list`; the tools load in the next Claude Code session.

### Configuring other MCP clients

Clients that read an `mcpServers` JSON file (such as Claude Desktop's
`claude_desktop_config.json`) take the same command:

```json
{
  "mcpServers": {
    "openxr-simulator": {
      "command": "python",
      "args": ["C:/path/to/OpenXR-Simulator/mcp-server/openxr_simulator_mcp.py"]
    }
  }
}
```

### Using the simulator without activating it

Activation replaces the machine-wide OpenXR runtime, which you may not want on a
machine that also drives a real headset. Instead, set `XR_RUNTIME_JSON` for the
application you are testing, and only that process uses the simulator:

```powershell
$env:XR_RUNTIME_JSON = "C:\path\to\OpenXR-Simulator\bin\openxr_simulator.json"
& "C:\path\to\YourOpenXRApp.exe"
```

The MCP server needs no extra setup for this; it finds the running runtime
through the shared data folder.

## Available Tools

The tools only report data while an OpenXR application is running on the
simulator. With no application running, they return an unknown session state.

### Capture

#### `capture_screenshot`
Capture a screenshot of the current OpenXR frame, as shown in the preview window.

Parameters:
- `eye`: Which eye view to capture ("both", "left", "right") - default: "both"
- `timeout`: Timeout in seconds - default: 5.0

#### `get_frame_info`
Get detailed information about the current frame including timing, resolution, format, and head tracking state.

#### `get_projection_log`
Return the recent projection-layer FOV, pose and image rect the application submitted through `xrEndFrame`, to compare with what the simulator published.

Parameters:
- `max_entries`: Number of recent entries - default: 20

#### `validate_stereo`
Capture the stereo preview and measure horizontal disparity between the eyes. Returns a verdict (`PASS`, `FAIL_NO_PARALLAX`, `FAIL_EXCESSIVE_PARALLAX`, or `INCONCLUSIVE_NO_FEATURES` when the centre of view is flat), the measured and expected disparity, and a hint.

Parameters:
- `timeout`: Timeout in seconds - default: 5.0

### Diagnose

#### `read_logs`
Read recent entries from the OpenXR Simulator log file.

Parameters:
- `lines`: Number of recent log lines to read - default: 100
- `filter`: Optional regex pattern to filter log lines

#### `get_diagnostics`
Get comprehensive diagnostic information including session state, frame count, swapchain info, and errors.

#### `diagnose_issue`
Analyze potential OpenXR issues based on symptoms.

Parameters:
- `symptoms`: Description of the issue or symptoms (required)

#### `get_session_state`
Get the current OpenXR session state (IDLE, READY, SYNCHRONIZED, VISIBLE, FOCUSED, STOPPING, EXITING).

#### `get_head_tracking`
Get the current head tracking state including position and orientation.

#### `clear_logs`
Clear the OpenXR Simulator log file to start fresh.

### Flicker detection

#### `get_flicker_status`
Read the continuous detector attached to the final composed preview: layer-continuity and pixel anomaly counters, plus a contact sheet from the latest incident when one exists.

Parameters:
- `include_images`: Include the incident contact sheet - default: true
- `max_frames`: Maximum contact-sheet frames (2-12) - default: 8

#### `capture_flicker_window`
Capture consecutive fully composed preview frames now, bypassing the Mirror Rate cap, even if the automatic detector has not fired. Returns a contact sheet and per-frame swapchain release and freshness metadata.

Parameters:
- `timeout`: Seconds to wait (1-15) - default: 5.0
- `max_frames`: Maximum contact-sheet frames (2-12) - default: 12

#### `get_ui_flicker_status`
Read UI-only quad submission, GPU readback, fresh composition, cached recomposition, and omission counters. This ignores world motion. When an incident exists, optionally returns a contact sheet cropped to the projected UI rectangle in both eyes.

Parameters:
- `include_images`: Include the incident contact sheet - default: true
- `max_frames`: Maximum contact-sheet frames (2-12) - default: 8

#### `capture_ui_flicker_window`
Force a rolling pre/post UI-only capture even when automatic thresholds remain clean. Use it when someone reports UI flicker but whole-frame detection is clean.

Parameters:
- `timeout`: Seconds to wait for a valid quad capture (1-15) - default: 5.0
- `max_frames`: Maximum contact-sheet frames (2-12) - default: 12

### Control

#### `set_head_pose`
Set the simulated head pose. A non-identity pose, especially roll, surfaces coordinate-system and quaternion-handedness bugs that an identity pose hides.

Parameters:
- `x`, `y`, `z`: Head position in meters - defaults: 0.0, 1.7, 0.0
- `yaw_deg`, `pitch_deg`: Orientation in degrees - default: 0.0
- `roll_deg`: Head tilt in degrees; omit to leave it unchanged

#### `set_fov`
Override the per-eye FOV with asymmetric values. The default FOV is symmetric, which hides projection-matrix bugs that appear with a real headset's asymmetric lenses.

Parameters:
- `left_eye`, `right_eye`: Objects with `left_deg`, `right_deg`, `up_deg`, `down_deg`
- `clear`: Revert to the symmetric default - default: false

#### `set_ipd`
Override the eye separation. At 0 mm both eyes should render identical images; 80 mm checks large-IPD behavior.

Parameters:
- `ipd_mm`: IPD in millimeters (0-100)
- `clear`: Revert to 64 mm - default: false

#### `set_headset_profile`
Apply a named headset preset (FOV and IPD together), using the per-eye frustum each headset's runtime reports.

Parameters:
- `name` (required): `quest2`, `quest3`, `questpro`, `index`, `vivepro2`, `reverbg2`, `psvr2`, `pico4`, `beyond`, or `generic` / `default` / `clear` to revert

#### `enable_anaglyph_preview`
Composite both eyes into one red/cyan image in the preview window. Converged stereo looks mostly gray; broken IPD or swapped eyes show strong red/cyan ghosting.

Parameters:
- `enabled`: default: true

#### `enable_pose_sweep`
Oscillate head yaw, pitch and roll on sine waves at different phases, so handedness and axis-flip bugs show up as the world wobbling the wrong way within seconds.

Parameters:
- `enabled`: default: true
- `yaw_amp_deg`: default: 30.0
- `pitch_amp_deg`: default: 15.0
- `roll_amp_deg`: default: 15.0
- `freq_hz`: default: 0.25

## File Locations

The MCP server and runtime exchange files in `%LOCALAPPDATA%\OpenXR-Simulator\`.
Set `SIMXR_DATA_DIR` for both the runtime and the MCP server to use another
folder, for example to run instances side by side.

- `openxr_simulator.<pid>.log` - Runtime log, one per process (the server reads the newest)
- `screenshot_request.json` / `screenshot.bmp` - Screenshot request and result
- `runtime_status.json`, `frame_info.json` - Current frame status
- `flicker_status.json`, `flicker_capture_request.json`, `flicker_incidents\` - Whole-frame flicker status, forced captures and evidence
- `ui_flicker_status.json`, `ui_flicker_capture_request.json`, `ui_flicker_incidents\` - UI-only flicker status, forced captures and evidence
- `head_pose_command.json`, `fov_command.json`, `ipd_command.json`, `headset_profile_command.json`, `anaglyph_command.json`, `pose_sweep_command.json` - Control commands the runtime consumes
- `projection_log_dump_request` / `projection_log.json` - Projection log request and result

## How It Works

1. The MCP server communicates with the OpenXR Simulator runtime through file-based IPC
2. Screenshot and capture requests are made by writing a file that the runtime watches for
3. The runtime captures the preview backbuffer and saves it as a BMP file
4. Frame and flicker status are periodically written to JSON files for diagnostics
5. Control tools write command files that the runtime polls for and consumes

## Troubleshooting

### Screenshots not capturing
- Ensure an OpenXR application is actively rendering frames
- Check that the application is using the simulator: either it is the active runtime, or `XR_RUNTIME_JSON` points at `openxr_simulator.json` for that process
- Verify the simulator data directory exists: `%LOCALAPPDATA%\OpenXR-Simulator\`

### Log file not found
- The log file is created when the OpenXR Simulator runtime first runs
- Start an OpenXR application to generate log entries

### MCP connection issues
- Ensure the dependencies are installed: `pip install -r requirements.txt`
- Check the Python version is 3.10 or higher
- In Claude Code, run `claude mcp list` to see whether the server connected
