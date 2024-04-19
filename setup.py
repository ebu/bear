from setuptools import setup, find_packages

setup(
    name="bear",
    description="Binaural EBU ADM Renderer",
    version="0.0.1",
    install_requires=[
        "ear~=2.1",
        "importlib_resources",
        "numpy",
    ],
    packages=find_packages(),
    extras_require={
        "test": ["pytest"],
        "dev": ["flake8", "flake8-black"],
    },
    entry_points={
        "console_scripts": [
            "bear-render = bear.render_cli:main",
        ]
    },
)
