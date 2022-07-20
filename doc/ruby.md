# Installing Ruby and Gems

First, install Ruby using `rbenv`, by calling the following script:
```
scripts/installRuby.sh
```
- this will build and install Ruby in `.rbenv`, using `rbenv` and `ruby-build`
- you will need to re-run `source environ.sh` afterward

Then install gems by running
```
bundle install
```
If all went well, you can now run the `*.rb` executables
