# Pixel Streaming Credential Provider

A Windows Credential Provider implementation that enables programmatic user authentication and session creation for pixel streaming scenarios. This project provides automatic, headless Windows logon capabilities for cloud gaming, remote desktop, and virtual desktop infrastructure (VDI) applications.

## 🎯 Project Goals

This project aims to solve a critical challenge in pixel streaming: **creating interactive Windows user sessions programmatically without user interaction**. Traditional approaches require users to manually enter credentials at the Windows logon screen, which breaks the seamless experience needed for cloud gaming and remote application delivery.

### What We're Building

A custom Windows Credential Provider that:
- ✅ Enables automatic user logon without manual credential entry
- ✅ Creates interactive desktop sessions suitable for GPU-accelerated applications
- ✅ Integrates with pixel streaming capture services
- ✅ Supports both local and domain user accounts
- ✅ Provides secure credential handling using Windows DPAPI
- ✅ Works across Windows 10, Windows 11, and Windows Server editions

## 🔧 Technical Approach

### Core Technology: Windows Credential Provider Framework

We're implementing the Windows Credential Provider COM interfaces to integrate directly with the Windows authentication system. This approach works **with** Windows security rather than trying to bypass it.

```cpp
// Key COM interfaces we implement
class CPixelStreamProvider : public ICredentialProvider,
                             public ICredentialProviderSetUserArray
{
    // Integrates with Windows LogonUI
};

class CPixelStreamCredential : public ICredentialProviderCredential2
{
    // Handles actual authentication
};
```

### How It Works

1. **Service Triggers Authentication**
   ```
   External Request → Service API → Store Encrypted Credentials → Set Auto-logon Flag
   ```

2. **Credential Provider Auto-logon**
   ```
   Windows LogonUI → Load Our Provider → Detect Auto-logon Flag → Submit Credentials
   ```

3. **Session Creation & Streaming**
   ```
   Windows Creates Session → Launch Capture Process → Start Pixel Streaming
   ```

### Key Components

#### 1. **Credential Provider DLL** (C++)
- Implements Windows authentication interfaces
- Handles secure credential storage and retrieval
- Provides automatic logon without user interaction
- Integrates with Windows LogonUI process

#### 2. **Session Management Service** (C++/Python)
- Receives session creation requests via API
- Manages credential lifecycle
- Monitors session state
- Launches pixel streaming capture

#### 3. **Security Layer**
- Credentials encrypted using Windows DPAPI
- One-time use auto-logon tokens
- Audit logging for compliance
- Secure communication channels

## 📋 Requirements

### Development Environment
- Windows 10/11 SDK (10.0.19041 or later)
- Visual Studio 2019 or later
- C++17 compiler support
- Windows Driver Kit (WDK) for testing

### Runtime Requirements
- Windows 10 version 1903+ or Windows Server 2019+
- Administrative privileges for installation
- LocalSystem service account for session management

## 🏗️ Architecture

```
┌─────────────────────────┐     ┌─────────────────────────┐
│   Client Application    │     │   Pixel Stream Service  │
│  (Requests Session)     │────▶│  (Orchestrates Login)   │
└─────────────────────────┘     └───────────┬─────────────┘
                                            │
                                            ▼
┌─────────────────────────┐     ┌─────────────────────────┐
│  Credential Provider    │◀────│   Secure Credential     │
│  (Auto-logon Module)    │     │      Storage            │
└───────────┬─────────────┘     └─────────────────────────┘
            │
            ▼
┌─────────────────────────┐     ┌─────────────────────────┐
│    Windows LogonUI      │────▶│   New User Session      │
│  (Authentication)       │     │   (Desktop Created)     │
└─────────────────────────┘     └─────────────────────────┘
```

## 🚀 Planned Features

### Phase 1: Core Implementation ✅
- [ ] Basic credential provider structure
- [ ] COM interface implementation
- [ ] Auto-logon capability
- [ ] Secure credential storage

### Phase 2: Integration 🚧
- [ ] Service API for session requests
- [ ] WebSocket communication layer
- [ ] Session lifecycle management
- [ ] Pixel streaming launcher integration

### Phase 3: Security & Scaling 📋
- [ ] Multi-factor authentication support
- [ ] Group Policy integration
- [ ] Load balancing across session hosts
- [ ] Comprehensive audit logging

### Phase 4: Advanced Features 🔮
- [ ] Biometric authentication
- [ ] Smart card support
- [ ] Azure AD integration
- [ ] Kubernetes operator for cloud deployment

## 🔐 Security Considerations

This project handles sensitive authentication data and must be implemented with security as a top priority:

1. **Credential Protection**
   - All passwords encrypted using Windows DPAPI
   - Credentials stored in protected registry locations
   - Automatic cleanup after use

2. **Access Control**
   - Service runs as LocalSystem
   - Registry ACLs restrict access
   - API authentication required

3. **Audit Trail**
   - All authentication attempts logged
   - Windows Event Log integration
   - Compliance with security standards

## 🏗️ Building from Source

### Prerequisites

1. Install Visual Studio 2019+ with:
   - Desktop development with C++
   - Windows 10 SDK
   - ATL/MFC components

2. Clone the repository:
   ```bash
   git clone https://github.com/SpeedxDevil/PixelStreaming.CredentialProvider.git
   cd pixel-stream-credential-provider
   ```

3. Build the solution:
   ```bash
   msbuild PixelStreaming.CredentialProvider.sln /p:Configuration=Release /p:Platform=x64
   ```

### Installation

1. Copy the built DLL to System32:
   ```bash
   copy x64\Release\PixelStreaming.CredentialProvider.dll C:\Windows\System32\
   ```

2. Register the credential provider:
   ```bash
   regsvr32 C:\Windows\System32\PixelStreaming.CredentialProvider.dll
   ```

3. Configure the service (details in docs/)

## 📚 Documentation

Detailed documentation will be provided in the `/docs` directory:

- [Architecture Overview](docs/architecture.md)
- [API Reference](docs/api-reference.md)
- [Security Model](docs/security.md)
- [Deployment Guide](docs/deployment.md)
- [Troubleshooting](docs/troubleshooting.md)

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## ⚠️ Disclaimer

This software modifies Windows authentication behavior. Improper use can:
- Lock users out of systems
- Create security vulnerabilities
- Violate compliance requirements

**Always test in isolated environments first!**

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Microsoft Windows SDK documentation
- Windows Credential Provider samples
- The security research community
- Contributors and testers

## 🔗 Related Projects

- [Pixel Streaming PoC](https://github.com/SpeedxDevil/pixel-streaming) - The main pixel streaming implementation

## 📞 Contact

- Project Lead: [Your Name]
- Email: your.email@example.com
- Discord: [Project Discord Server]
- Issues: [GitHub Issues](https://github.com/SpeedxDevil/PixelStreaming.CredentialProvider/issues)

---

**Note**: This is an active development project. APIs and interfaces may change. Not recommended for production use until version 1.0 release.
