# Paths & files

$source_path = 'Source\'
$deploy_path = 'Deploy\'
$source_files = "$(Get-ChildItem -File -Include *.cpp -Recurse $source_path\)"

# Compile & link options

$compile_opts = [String]::Join(' ',
	# include paths
	"/I'$source_path'",
	# preprocessing
	'/D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE"',
	# optimization
	'/O2 /Oi',
	# code generation
	'/EHsc /fp:precise /GS /GL /Gy /Gd',
	# language
	'/Zc:wchar_t /Zc:inline',
	# warning
	'/W3 /WX-',
	'')
$link_opts = [String]::Join(' ',
	'/DYNAMICBASE "gdiplus.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib"',
	'/SUBSYSTEM:CONSOLE',
	'/MACHINE:X64',
	'/OUT:Renderer.exe',
	'')

# Build

$vcvars = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat'
cmd /e:on /c """$vcvars"" && cd $deploy_path && cl $compile_opts $source_files /link $link_opts"
Remove-Item "$deploy_path\*.obj"
