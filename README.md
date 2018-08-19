# Whais PHP Ext

htttp://www.whais.net (to be available in the next days)

This project is a simple extension to facilitate users of PHP (version 7.2.6 and higher) interact with a Whais server. 

This extension is very much work in progress. It does provide the basic functionalty to connect to a Whais database server, execute procedures and fetch results. However, its functionality was tested in a limited way. 

## Instalation

To use this module, it has to be built from source code. It is developed for PHP version 7.2.6 and newer. The module is relying also an the Whais’ C client library in order to function. The build of this module is done as any other PHP module. Please refer to PHP documentation how to properly use the commands bellow: 

```
git clone git@github.com:iulianpopa1981/whais_php_ext.git 

phpzie 

./configure 

make 

make install 
```

Also, please make sure to update the config.m4 file after clonning this so to properly point to the location of the Whais client library.


## Contributing

Everyone is welcome to contribute in any way to improve this program. Even if you have just an idea how to this, please do share it here.  Otherwise:  

1. Fork it!  

2. Recall to have fun while you do all next steps. Chances are you are going to change the world a little bit.

3. Create your feature branch: `git checkout -b my-new-feature`  

4. Commit your changes: `git commit -am 'Add some feature'`  

5. Push to the branch: `git push origin my-new-feature` 

6. Submit a pull request :D 


## History

19th August 2018 - First version publicly available 

## License
MIT License

Copyright (c) 2018 Iulian Popa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
