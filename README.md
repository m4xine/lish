# Li(sp)sh(ell)

## Can I use this?

Probably not! This is very experimental and I doubt it'd amount to much. It lacks a lot of the features most shells have that I'll most likely not have time to add.

## Examples

```lisp
#!/bin/lish

(deval CC   'cc')
(deval OUT  './lish')
(deval ARGS '-std=gnu99')

(if (exists OUT) ('rm' OUT))

(CC '-o ' OUT ' src/lish.c ' ARGS)
```

```lisp
#!/bin/lish

(defun dog-say (msg) 
  (echo 'woof woof ' msg))

(dog-say 'i am a dog.')
```

There are more examples [here](https://github.com/m4xine/lish/tree/main/tests)!

## Future plans

I may add these in the future:

  + Records
  + More built-in functions 
  + Comprehensive errors
  + Error recovery