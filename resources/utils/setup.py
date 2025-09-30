from setuptools import setup, find_packages

with open("requirements.txt") as f:
    install_requires = f.read().strip().split("\n")

setup(
    name="aqnwb-utils",
    version="0.1.0",
    packages=find_packages(),
    py_modules=["aqnwb_utils", "generate_spec_files", "schematype_to_aqnwb"],
    install_requires=install_requires,
    entry_points={
        "console_scripts": [
            "aqnwb-utils=aqnwb_utils:main",
        ]
    },
)
