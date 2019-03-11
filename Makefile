all:
	python3 setup.py build

clean:
	rm -rf build
	rm -rf MANIFEST
	rm -rf dist
	rm -f *.pyc
	rm -rf __pycache__
	rm -rf package/__pycache__
	rm -rf package/*.cpp
