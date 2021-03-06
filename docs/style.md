## Coding guidelines. (DO NOT IGNORE)

### Intro
Generally speaking, just try to match the existing code and the [Linux kernel guidelines](https://kernel.org/doc/html/latest/process/coding-style.html), and if you think you understand the guidelines well enough, try to make other code follow the style where it doesn't. That said, here are some other rules for writing code for the engine.

### Include statement organization
We use the old C99 names for headers. Include statements should be ordered as follows, with one blank line between each set of includes:
- Standard headers (`stdlib.h`, `string.h`, `errno.h`, etc)
- Platform-specific headers
- External library headers (stuff in the `deps/` folder)
- In quotation marks rather than angle braces, headers in the `include/purpl/` folder
- Headers in the current folder

### Standard library usage
We use only the following from the C++ standard library, and nothing more:
- Classes
- Namespaces (only really `purpl`, nested namespaces are stupid anyway)
- `new`/`delete`, but _ONLY_ for classes, or else bad things will happen to whoever disobeys this rule

Otherwise, we use the C99 standard library (`calloc`/`free` for memory, `fopen`/`fprintf`/`fread`/`fwrite`/`fclose` for files, et cetera. If you don't know C, better start Googling now), which is a mandatory part of any standard-conformant implementation, and also usually present in non-conformant ones as well. I know this seems a waste, but this project was originally going to be in C, and that only changed because of how awesome ImGui is, and also because of namespaces/classes.

### Conditional statements
We apply the following rules for conditionals (`if/else`, `for`, `while`, etc):
- Variables come before constants and macros, and when comparing variables, the variable whose value is being checked comes first
- When it comes to `if` statements that check for errors, unless the return value is needed, do not use a separate variable to check for the error, and do not check whether something succeeded, check if it failed, like so:
```c++
if (!some_func(foo))
	/* Clean up and return a default value or something indicative of an error */

/* What should normally happen afterward goes here */
```
- In for loops, the preferred style is as follows (with the variable declared before the loop, which means that you only need to declare multiple variables for _nested_ `for` loops. Also, use brackets when there is more than one statement, even if it's not strictly necessary):
```c++
/* Variable definitions */
uint i;

for (i = 0; i < something; i++)
	/* Iterate over something */

for (i = 0; i < strlen(foo); i++) {
	if (foo[i] == bar)
		/* Do something */
}
```
- Don't check for errors that do not affect anything the engine needs to do or have happen, because there is no need. This is extremely rare however, so if you think that the error isn't worth worrying about, check if that's true first

#### Conditionally included code
- Only use for platform-specific stuff, and only if it's simple and/or small enough not to go in a separate folder/function, which is rather unlikely in most cases (but don't worry as much for Vulkan-related files, as it significantly reduces code duplication)
- If multiple different platforms are being handled in the same place, start with `#ifdef` rather than `#if`, it just looks better in my opinion, which means it's how things are to be done

### Time
If you're feeling in a particularly humorous mood, it's completely acceptable to write the time as SAUE format or the year in UE format (__S__ econds __A__ fter __U__ NIX __E__ xists and __U__ NIX __E__ xists, respectively). Also the format for writing dates is DD/MM/YYYY, otherwise YYYY/MM/DD, but please, under no circumstances _ever_, MM/DD/YYYY, because that's just insane.

<sub>_Written by MobSlicer152, last updated 01/11/50 :)_</sub>
