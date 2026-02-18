# zjson
C++ JSON API, wrapper of Jansson C library

The difference with the various C++ JSON alternatives is:
- while it defaults to c++20 it also supports old-ish C++ standards (tested with C++14).
- it is a wrapper of the C library Jansson, which is mature and widely used (very permissive MIT license).
- used for quite some time in production code on Windows, so it is stable and well tested.
- simple source (1 header and 1 cpp), easy to understand and modify/adjust if you need it.

# Usage
    json::Object jObj{{"field", "value"}, {"field2", 123}};
    jObj["field3"] = json::Array{1, 2, std::string{}};
    ioStream << jObj;
    ioStream >> jObj;
    std::string str = json::jsonToString(jObj);
    auto jObj2 = json::strToObject(str);

See the demo file for more examples.

# Building
Simle CMake build. Set JANSSON_ROOT if CMAKE cannot find Jansson on its own.
Use ZJSON_BUILD_SHARED option to build a shared library, otherwise a static library will be built.
If using shared Jansson lib, make sure it is available for your executable.

Any problems, questions or suggestions are welcome.

# License
MIT License

# Credits
- Jansson library: https://github.com/akheron/jansson
- Bing Technologies P/L: https://www.bingmail.com.au/
