<div align="center">

# QChat

### Your Private, Powerful AI Chat Client

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-6.5+-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![C++17](https://img.shields.io/badge/C++-17-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey)](https://github.com/axeprpr/qchat/releases)

<br/>

**QChat** is a modern, privacy-first AI chat client that puts you in control. Connect your own API keys, choose your models, and enjoy features that go far beyond basic chatting.

*No accounts. No data collection. No subscriptions. Just your keys, your models, your data.*

<br/>

[Features](#-features) &bull; [Quick Start](#-quick-start) &bull; [Screenshots](#-screenshots) &bull; [Build](#-build-from-source) &bull; [Roadmap](#-roadmap)

</div>

---

## Why QChat?

Most AI chat tools are either:
- **Cloud-locked** — your conversations live on someone else's server
- **Feature-poor** — basic chat with no deep thinking, no file analysis, no real power
- **Single-provider** — locked to one AI company

**QChat breaks free from all of these.** It's a beautiful, native desktop app that connects to *any* AI provider — OpenAI, Anthropic Claude, Google Gemini, DeepSeek, or local models via Ollama. With advanced features like **deep thinking mode**, **document analysis**, and **streaming markdown rendering**, QChat is the most powerful private AI client you can get.

---

## Features

### Multi-Provider Intelligence
Connect to the world's best AI models — all from one unified interface:

| Provider | Models | Special Features |
|----------|--------|-----------------|
| **OpenAI** | GPT-4o, GPT-4 Turbo, o1, o3-mini | Vision, Reasoning |
| **Anthropic** | Claude Opus 4.6, Sonnet 4.6, Haiku 4.5 | Extended Thinking |
| **DeepSeek** | DeepSeek-V3, DeepSeek-R1 | Deep Reasoning |
| **Google** | Gemini 2.5 Pro, Gemini 2.5 Flash | Multi-modal |
| **Ollama** | Llama 3.3, Qwen 3.5, Mistral, Phi-4... | 100% Local & Offline |

### Deep Thinking Mode
Toggle deep thinking to see your AI's reasoning process in real-time. Works with:
- OpenAI o1/o3 reasoning models
- DeepSeek-R1 chain-of-thought
- Claude's extended thinking
- Local reasoning models via Ollama (DeepSeek-R1, QwQ, etc.)

### Document Analysis
Drag & drop files directly into the chat:
- **Code files** — Python, JavaScript, TypeScript, C++, Rust, Go, Java, and 20+ languages
- **Data files** — JSON, CSV, XML, YAML
- **Text documents** — Markdown, plain text, HTML
- Smart truncation for large files with context preservation

### Rich Markdown Rendering
- Full markdown with **bold**, *italic*, ~~strikethrough~~
- Syntax-highlighted code blocks (Python, JS, C++, and more)
- Tables, lists, blockquotes, horizontal rules
- Clickable links and inline code

### Beautiful Native UI
- Built with Qt 6 and Fluent Design language
- Smooth animations and transitions
- Dark and light themes with one-click toggle
- Adjustable font size
- Responsive sidebar with conversation management

### Privacy First
- **Zero telemetry** — no data leaves your machine except API calls
- **Local storage** — conversations saved locally in JSON
- **Your keys** — bring your own API keys, stored locally
- **Offline capable** — works fully offline with Ollama models

---

## Quick Start

### Download

Prebuilt packages are published automatically on each `v*` tag:

- **GitHub Releases**: https://github.com/axeprpr/qchat/releases
- Windows: `qchat-windows-x64-vX.Y.Z.exe`
- Linux: `qchat-linux-x64-vX.Y.Z.AppImage`
- macOS (Universal): `qchat-macos-universal-vX.Y.Z.dmg`
- Each artifact includes a matching `.sha256.txt` checksum file.

If you don't find a package for your platform yet, build from source below.

### Configure

1. Launch QChat
2. Click the **Settings** icon (top-right)
3. Expand your preferred provider
4. Enter your API key and (optionally) customize the base URL
5. Start chatting!

**For local models:** Install [Ollama](https://ollama.ai), pull a model (`ollama pull llama3.3`), and select "Ollama" as your provider. No API key needed.

---

## Screenshots

<div align="center">

*Screenshots coming soon — the UI features a modern dark theme with sidebar navigation, streaming responses, and deep thinking visualization.*

</div>

---

## Build from Source

### Prerequisites

- **Qt 6.5+** — [Download Qt](https://www.qt.io/download-qt-installer)
  - Required modules: Qt Quick, Qt Network, Qt SVG
- **CMake 3.21+**
- **C++17 compiler** — MSVC 2019+, GCC 11+, or Clang 13+

### Windows

```powershell
git clone https://github.com/axeprpr/qchat.git
cd qchat
scripts\build_windows.bat
```

The script auto-detects your Qt installation and builds a deployable package in `build-win/deploy/`.

### Linux / macOS

```bash
git clone https://github.com/axeprpr/qchat.git
cd qchat
./scripts/build_linux.sh
```

### Manual Build

```bash
git clone https://github.com/axeprpr/qchat.git
cd qchat

# Optional: Include FluentUI for enhanced visuals
git clone --depth 1 https://github.com/zhuzichu520/FluentUI.git third_party/FluentUI

mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

---

## Architecture

```
qchat/
├── CMakeLists.txt              # Build system
├── CMakePresets.json            # Build presets
├── src/
│   ├── main.cpp                # Entry point
│   ├── core/
│   │   ├── ChatManager.*       # Central orchestrator
│   │   ├── ModelProvider.h     # Abstract provider interface
│   │   ├── OpenAIProvider.*    # OpenAI / DeepSeek / Gemini (compatible API)
│   │   ├── ClaudeProvider.*    # Anthropic Claude (native API)
│   │   ├── OllamaProvider.*   # Local Ollama models
│   │   ├── MessageModel.*     # Chat message data model
│   │   ├── ConversationListModel.*  # Conversation management
│   │   ├── SettingsManager.*  # Persistent settings
│   │   ├── DocumentParser.*   # File parsing engine
│   │   ├── MarkdownHelper.*   # Markdown → HTML renderer
│   │   └── ThinkingParser.*   # <think> tag parser
│   └── qml/
│       ├── Main.qml           # App window & layout
│       ├── ChatView.qml       # Message list
│       ├── MessageBubble.qml  # Individual message
│       ├── ThinkingBlock.qml  # Reasoning visualization
│       ├── Sidebar.qml        # Conversation list
│       ├── SettingsDialog.qml # Settings panel
│       ├── WelcomeView.qml    # Welcome screen
│       ├── ProviderConfig.qml # Provider setup
│       ├── DocumentChip.qml   # File attachment chip
│       └── components/        # Reusable components
├── resources/                  # Icons, fonts, assets
└── scripts/                    # Build scripts
```

---

## Roadmap

- [ ] **Image generation** — DALL-E 3, Stable Diffusion integration
- [ ] **Vision/image input** — Send images to vision models
- [ ] **Voice input/output** — Speech-to-text and TTS
- [ ] **PDF parsing** — Native PDF document analysis
- [ ] **Plugin system** — Extend with custom tools
- [ ] **MCP support** — Model Context Protocol for tool use
- [ ] **Export** — Export conversations to Markdown, PDF, HTML
- [ ] **Multi-language UI** — i18n support
- [ ] **Prompt library** — Save and reuse prompt templates
- [ ] **RAG integration** — Local knowledge base with vector search

---

## Tech Stack

- **Qt 6 + QML** — Native cross-platform UI framework
- **C++17** — High-performance backend
- **FluentUI** — Microsoft Fluent Design components for Qt
- **CMake** — Modern build system with presets

---

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

---

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

---

<div align="center">

**Built with love using Qt 6 and the world's best AI models.**

[Report Bug](https://github.com/axeprpr/qchat/issues) &bull; [Request Feature](https://github.com/axeprpr/qchat/issues)

</div>
