"""Build a portable ZIP package for JsonParser."""
import os
import shutil
import zipfile

BUILD_DIR = r"E:\jsonparser\qt\build"
QT_DIR = r"D:\Qt6.5.3_vs2019_x64"
OUTPUT_DIR = r"E:\jsonparser\qt\dist"
APP_NAME = "JsonParser"

def collect_files():
    """Collect all runtime files needed by the application."""
    files = []

    # Main executable and icon
    files.append(("JsonParser.exe", os.path.join(BUILD_DIR, "JsonParser.exe")))
    files.append(("app.ico", os.path.join(r"E:\jsonparser\qt", "app.ico")))

    # Qt DLLs
    for dll in ["Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll"]:
        path = os.path.join(BUILD_DIR, dll)
        if os.path.exists(path):
            files.append((dll, path))

    # ANGLE/D3D compiler
    d3d_path = os.path.join(BUILD_DIR, "d3dcompiler_47.dll")
    if os.path.exists(d3d_path):
        files.append(("d3dcompiler_47.dll", d3d_path))

    # Qt plugins
    plugin_dirs = {
        "platforms": ["qwindows.dll"],
        "styles": ["qwindowsvistastyle.dll"],
        "imageformats": ["qico.dll", "qsvg.dll"],
        "iconengines": ["qsvgicon.dll"],
        "generic": ["qtuiotouchplugin.dll"],
        "tls": ["qcertonlybackend.dll", "qschannelbackend.dll"],
    }

    for subdir, dlls in plugin_dirs.items():
        for dll in dlls:
            path = os.path.join(BUILD_DIR, subdir, dll)
            if os.path.exists(path):
                files.append((f"{subdir}/{dll}", path))

    return files

def build_zip():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    zip_path = os.path.join(OUTPUT_DIR, f"{APP_NAME}-1.0.0-Portable.zip")

    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zf:
        for rel_path, abs_path in collect_files():
            arc_name = f"{APP_NAME}/{rel_path}"
            print(f"  Adding: {arc_name}")
            zf.write(abs_path, arc_name)

    size_mb = os.path.getsize(zip_path) / (1024 * 1024)
    print(f"\nPackage created: {zip_path}")
    print(f"Size: {size_mb:.1f} MB")

if __name__ == "__main__":
    build_zip()