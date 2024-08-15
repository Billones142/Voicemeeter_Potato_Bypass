zig cc -o ".\VoicemeeterBypass.exe" ".\src\VoicemeeterBypass.c"
if ($?) {
  Remove-Item "VoicemeeterBypass.pdb"
}