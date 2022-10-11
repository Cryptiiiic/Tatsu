rm -rf dist/*
poetry build
if [[ $(command -v pip3) ]]
then
  pip3 install $(ls dist/*.tar.gz | xargs)
else
  python3 -m pip install $(ls dist/*.tar.gz | xargs)
fi
