all:
	@echo "usage: make [build][install][clean][remove]"

build:
	python3 setup.py build

install:
	python3 setup.py install

dist:
	python3 setup.py sdist
up:
	twine upload dist/*

up-test:
	twine upload dist/* -r testpypi
	pip install -i https://testpypi.python.org/pypi graphyti

clean:
	rm -rf build
	rm -rf MANIFEST
	rm -rf dist
	rm -f *.pyc
	rm -rf __pycache__
	rm -rf graphyti/__pycache__
	rm -rf graphyti/*.cpp
	rm -rf graphyti/Exceptions/__pycache__

remove:
	rm -rf /usr/local/lib/python3.4/dist-packages/graphyti*
