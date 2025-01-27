# How to contribute 

1. create a branch 
2. write your code 
3. pull request

# syntax

```cpp
start {
    intbox x = 5;
    intbox y = 10;
    intbox result;

    out << "Enter a number: " ;
    in >> x ;

    result = x + y;

    out << "The result is: " << result ;

    if (result > 10) {
        out << "The result is greater than 10." ;
    } else {
        out << "The result is 10 or less." ;
    }

    close
}
```

# Usage

## if cleanup needed
 `rm -rf build`

## run the program
`cd build`

`cmake ..`

`make `

`/kat_compiler ../tests/test.kat`
