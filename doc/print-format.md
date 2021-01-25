### Print Format

This file provides support for printing a formatted expression to a sized buffer. Any statement must be written like this

```
print_handler("This is a format statement {flags:width:option} ", arguments...)
```

Note that the colon characters and end bracket is optional, but strongly suggested. 

| Expression | Type | Description |
|:-:|:-:|:-:|
|<|Type|Left aligned the print in the given width field|
