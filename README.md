# BOF WinRM client
Cobalt Strike BOF that implements a WinRM shell client using Windows APIs.

Example:

```
beacon> winrm-client --host ws2 --cmd "whoami"

[+] host called home, sent: 6787 bytes
[+] received output:
thedomain\user1
```
Blog post: https://falconforce.nl/exploring-winrm-plugins-for-lateral-movement

References:
- Microsoft official documentation about WinRM API headers: https://learn.microsoft.com/en-us/windows/win32/api/_winrm/#enumerations
- Microsoft WinRM client shell example: https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/sysmgmt/winrm
