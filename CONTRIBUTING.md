# Contributing to Pixel Stream Credential Provider

First off, thank you for considering contributing to the Pixel Stream Credential Provider! It's people like you that make this project better for everyone.

## 🤝 Code of Conduct

This project and everyone participating in it is governed by our Code of Conduct. By participating, you are expected to uphold this code. Please report unacceptable behavior to [project email].

## 🚀 Getting Started

### Prerequisites

Before you begin, ensure you have:
- Windows 10/11 development machine
- Visual Studio 2019 or later
- Windows 10 SDK (10.0.19041+)
- Git for Windows
- Administrative privileges (for testing)

### Setting Up Your Development Environment

1. **Fork the repository**
   ```bash
   # Click "Fork" button on GitHub
   git clone https://github.com/SpeedxDevil/PixelStreaming.CredentialProvider.git
   cd PixelStreaming.CredentialProvider
   ```

2. **Create a branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/issue-description
   ```

3. **Set up Visual Studio**
   - Open `PixelStreaming.CredentialProvider.sln`
   - Ensure NuGet packages are restored
   - Build the solution to verify setup

## 📝 Development Guidelines

### Code Style

We follow the [Microsoft C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) with some modifications:

```cpp
// Class names: PascalCase with C prefix
class CPixelStreamProvider

// Method names: PascalCase
HRESULT Initialize();

// Member variables: underscore prefix
private:
    LONG _cRef;
    
// Constants: ALL_CAPS with underscores
const DWORD MAX_BUFFER_SIZE = 1024;

// Namespaces: PascalCase
namespace PixelStreamUtils
```

### Commit Messages

Follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

```
feat: add biometric authentication support
fix: resolve memory leak in credential serialization
docs: update API reference for session management
test: add unit tests for auto-logon flow
refactor: simplify registry access utilities
```

### Testing Requirements

1. **Unit Tests** (where applicable)
   ```cpp
   TEST(CredentialProvider, AutoLogonFlag) {
       // Test implementation
   }
   ```

2. **Integration Tests**
   - Test on Windows 10 and Windows 11
   - Test with local and domain accounts
   - Verify no memory leaks using Application Verifier

3. **Security Testing**
   - Run with standard user privileges
   - Verify credential encryption
   - Check for injection vulnerabilities

## 🐛 Reporting Bugs

### Before Submitting a Bug Report

1. Check the [existing issues](https://github.com/SpeedxDevil/PixelStreaming.CredentialProvider/issues)
2. Update to the latest version
3. Collect relevant information:
   - Windows version (`winver`)
   - Event Viewer logs
   - Debug output

### How to Submit a Bug Report

Create an issue with:
- Clear, descriptive title
- Steps to reproduce
- Expected vs actual behavior
- System information
- Relevant logs or screenshots

## 💡 Suggesting Enhancements

### Before Submitting an Enhancement

1. Check if it's already suggested
2. Ensure it aligns with project goals
3. Consider security implications

### How to Submit an Enhancement

Create an issue with:
- Use case description
- Proposed solution
- Alternative approaches considered
- Potential security impacts

## 🔨 Pull Request Process

### Before Submitting

1. **Update documentation**
   - API changes must update API docs
   - New features need user documentation
   - Update README if needed

2. **Add tests**
   - Unit tests for new functions
   - Integration tests for new features
   - Security tests for auth changes

3. **Check your code**
   ```bash
   # Run code analysis
   # Build in both Debug and Release
   # Test on clean Windows installation
   ```

### Submitting a Pull Request

1. **Fill out the PR template** completely
2. **Link related issues** using keywords (Fixes #123)
3. **Request reviews** from maintainers
4. **Respond to feedback** promptly
5. **Keep PR focused** - one feature/fix per PR

### After Your PR is Merged

- Delete your branch
- Pull latest changes to your fork
- Celebrate! 🎉

## 🔒 Security Vulnerabilities

**DO NOT** open public issues for security vulnerabilities!

Instead:
1. Email security@[projectdomain].com
2. Include detailed description
3. Provide proof of concept if possible
4. Allow time for patch before disclosure

## 📦 Release Process

Only maintainers can create releases:

1. Update version numbers
2. Update CHANGELOG.md
3. Create signed builds
4. Tag release in Git
5. Upload to GitHub releases
6. Update documentation

## 📚 Resources

### Windows Credential Provider Documentation
- [Credential Provider Technical Reference](https://docs.microsoft.com/en-us/windows/win32/secauthn/credential-providers-in-windows)
- [ICredentialProvider Interface](https://docs.microsoft.com/en-us/windows/win32/api/credentialprovider/nn-credentialprovider-icredentialprovider)

### COM Programming
- [COM Programming Introduction](https://docs.microsoft.com/en-us/windows/win32/com/the-component-object-model)
- [ATL Programming Guide](https://docs.microsoft.com/en-us/cpp/atl/atl-com-desktop-components)

### Security Best Practices
- [Windows Security Baselines](https://docs.microsoft.com/en-us/windows/security/threat-protection/windows-security-baselines)
- [Secure Coding Guidelines](https://docs.microsoft.com/en-us/windows/win32/secbp/best-practices-for-the-security-apis)

## ❓ Questions?

- Check our [FAQ](docs/faq.md)
- Join our [Discord server]
- Open a [discussion](https://github.com/SpeedxDevil/PixelStreaming.CredentialProvider/discussions)

Thank you for contributing to make Windows authentication better for pixel streaming! 🚀
