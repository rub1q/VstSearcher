# VstSearcher

## About
VstSearcher - is an advanced search 'engine' for VirtualStringTree (VirtualTreeView) that you can use in your personal projects.

## How to use
> Note, that you need a RAD Studio IDE with C++0x support (or higher). 

Firstly, initialize the searcher by calling the `Init()` method: 

```cpp
searcher::VstSearcher vstSearcher;
vstSearcher.Init(vtMain, edtSearchStr, lblAmntRows);
```

Then, specify columns in which you want searcher find matches:

```cpp
vstSearcher.SearchColumns << 0 << 1 << 2 << 4;
```

Searcher supports the following options
| Option | Description | By default |
| ------ | ------ | ------ |
| AUTO_EXPAND_NODES | Automatic expanding nodes where matches are found | YES |
| RELEVANT_SORT | Apply to the entire list sort by relevance | YES |
| START_SEARCH_AFTER_BUTTON_CLICK | Start the search after pressing the corresponding button | NO |

So you can specify any options you need. For example: 
```cpp
vstSearcher.SearchOptions >> SearchOption::RELEVANT_SORT;
```
You can also set limits for a search request (by default the min length is 2 and the max 128 symbols): 

```cpp
vstSearcher.SetMinRequestLength(2);
vstSearcher.SetMaxRequestLength(100);
// or
vstSearcher.SetRequestLimits(2, 100);
```
In order to get rid of the interface freezes on large lists adjust the input delay (default value is 300ms):
 ```cpp
 vstSearcher.SetInputDelay(1000); // in ms
 ```

## Licence 
[MIT License](https://github.com/rub1q/VstSearcher/blob/main/LICENSE)
