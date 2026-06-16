"""Package setup for Reconner."""

from pathlib import Path
from setuptools import find_packages, setup

readme = Path(__file__).parent / "README.md"
long_description = readme.read_text(encoding="utf-8") if readme.exists() else ""

setup(
    name="reconner",
    version="2.0.0",
    description="Automated, modular security reconnaissance orchestrator for authorized penetration testing.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="Reconner Contributors",
    author_email="",
    url="https://github.com/yourusername/reconner",
    license="MIT",
    packages=find_packages(),
    python_requires=">=3.10",
    install_requires=[
        "click>=8.0.0",
        "jinja2>=3.0.0",
        "reportlab>=3.6.0",
        "markdown>=3.4.0",
        "rich>=10.0.0",
    ],
    extras_require={
        "pdf": ["weasyprint>=57.0"],
        "dev": [
            "pytest>=7.0.0",
            "pytest-cov>=4.0.0",
            "black>=23.0.0",
            "ruff>=0.1.0",
            "pre-commit>=3.0.0",
            "mypy>=1.0.0",
        ],
    },
    entry_points={
        "console_scripts": [
            "reconner=reconner.cli:main",
        ],
    },
    include_package_data=True,
    package_data={
        "reconner": ["templates/*.j2"],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Information Technology",
        "Topic :: Security",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Operating System :: POSIX :: Linux",
        "Operating System :: MacOS",
    ],
    keywords="security reconnaissance pentest subfinder httpx nuclei gobuster whatweb",
)
