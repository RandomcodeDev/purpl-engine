## Coding guidelines. DO NOT IGNORE

#### Include statement organization
We use the old C99 names for headers. Include statements should be ordered as follows, with one blank line between each set of includes:
- Standard headers (`stdlib.h`, `string.h`, `errno.h`, etc)
- Headers in the current folder
- Platform-specific headers
- External library headers (stuff in the `deps/` folder)
- In quotation marks rather than angle braces, headers in the `include/purpl/` folder

#### Standard library usage
We use only the following from the C++ standard library, and nothing more:
- Classes
- Namespaces (only really `purpl`, nested namespaces are overrated anyway)
- `new`/`delete`, but _ONLY_ for classes

Otherwise, we use the C99 standard library, which is a mandatory part of any standard-conformant implementation, and also usually present in non-standard versions as well. I know this seems a waste, but this project was originally going to be in C, and that only changed because of how awesome ImGui is, and also because of namespaces/classes.
