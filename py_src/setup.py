import setuptools

setuptools.setup(
    name="vstruct",
    version="0.1",
    scripts=['vstruct/scripts/gen_header.py'],
    author="Joseph Lee",
    description="bit packing library",
    license='MIT',
    packages=['vstruct']
)
