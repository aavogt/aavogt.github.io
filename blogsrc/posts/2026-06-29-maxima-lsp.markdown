---
title: Maxima-lsp
tags: maxima
---

[The language server protocol](https://microsoft.github.io/language-server-protocol/overviews/lsp/overview/) the standard way to get deterministic help with code.
I use [neovim](http://neovim.io) to interact with R languageserver, clangd (c or c++), basedpyright (python), rust-analyzer or haskell-language-server. In any of those languages
I use the same keybindings to get information or modify code with the following operations:

1. looking up documentation
2. renaming an identifier
3. jump-to-definition
4. jump-to-use
5. factor out sub-expression (common subexpression elimination -- CSE)
6. inline (unCSE)
7. show inlay inferred types
8. jump to/show errors
9. complete a name adding imports if needed

[maxima](https://maxima.sourceforge.io/) didn't have a LSP. There are generic
language servers that can usually get 2,3,4,8 right. None of the
LSPs do 5&6 well. LSP is difficult to learn because there can be too many
layers written by different people that work together on a single task. For
example neovim used to use [coc.nvim](https://github.com/neoclide/coc.nvim) used 
[language server protocol](https://microsoft.github.io/language-server-protocol/overviews/lsp/overview/) to request
[haskell language server](https://github.com/haskell/haskell-language-server/) to ask
[retrie](https://hackage.haskell.org/package/retrie) to make changes. When it
doesn't work correctly, where is the mistake? Which layer should I ask for a
trace of what happened? Who has to change to fix the bug? Thankfully there's
common understanding of what the system is supposed to do.

I'll describe the process of putting [maxima-lsp](https://github.com/aavogt/maxima-lsp) together. At first I tried 
the [lsp](https://hackage-content.haskell.org/package/lsp)
package, because this is used by haskell-language-server and futhark. But I stopped at [this point](https://github.com/aavogt/maxima-lsp/blob/cb4aacea38891e0911e783c7dabf20447563fd67/main.hs#L55). I was surprised that the `lsp` package does not generate the `initialize` response which tells the editor (neovim) what it is allowed to send. The `LSP.Handlers` looks similar to the haskell `servant` library, which can generate a description of the API from the code that usually responds to that API. At least in principle the type checker could confirm that the client handles all responses it requests without requiring the client to handle all responses. The final version of the [`initialize` response is here for comparison](https://github.com/aavogt/maxima-lsp/blob/21e66dc0c73aabf46672056c83da18b9ede6cf2f/src/main.hs#L50). Starting at the main in each version, we'll look at how many steps it takes to reach the actual renaming logic:

With lsp library, `main` runs a `LSP.ServerDefinition` which configures their main loop with 8 different values/callbacks. The [staticHandlers](https://github.com/aavogt/maxima-lsp/blob/cb4aacea38891e0911e783c7dabf20447563fd67/main.hs#L55) includes the [handleRename2](https://github.com/aavogt/maxima-lsp/blob/cb4aacea38891e0911e783c7dabf20447563fd67/main.hs#L137) function that uses [lens-regex-pcre](https://hackage.haskell.org/package/lens-regex-pcre) to [apply the renaming](https://github.com/aavogt/maxima-lsp/blob/cb4aacea38891e0911e783c7dabf20447563fd67/main.hs#L148), and then callback's callback needs to see the results packaged up into a `Either (LSP.TResponseError LSP.Method_TextDocumentRename) (LSP.WorkspaceEdit LSP.|? LSP.Null)`. 

[In the current version](https://github.com/aavogt/maxima-lsp/blob/21e66dc0c73aabf46672056c83da18b9ede6cf2f/src/main.hs#L34), main sets up the server state and then has a loop to read requests and write the responses. [rename](https://github.com/aavogt/maxima-lsp/blob/21e66dc0c73aabf46672056c83da18b9ede6cf2f/src/main.hs#L208) pattern matches on the json Value described by [textDocument/rename](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#renameParams). Following that documentation has similar indirection to [lsp](https://hackage-content.haskell.org/package/lsp), except [lsp](https://hackage-content.haskell.org/package/lsp) adds an extra newtype around row/column/uri values. Another difference is that types like `interface RenameParams extends TextDocumentPositionParams, WorkDoneProgressParams {`  are missing from the latest code. In the first version, those constructors have to be named or the pieces extracted using functions like 
`position` indirectly referenced [here](https://hackage-content.haskell.org/package/lsp-types-2.4.0.0/docs/Language-LSP-Protocol-Types.html#t:RenameParams).
haskell-language-server doesn't treat the Has* classes/function specially. Essentially, the pattern `[json| _textDocument{uri} _position{line character} newName |]` can fit into short term memory, but the equivalent using [lsp](https://hackage-content.haskell.org/package/lsp) doesn't. The data definitions are spread across across many files and packages. I've collected the pieces which still takes me time and several passes to mentally fit together. Part of it is the use of names like `Language.LSP.Protocol.Internal.Types.TextDocumentIdentifier.TextDocumentIdentifier` instead of `Text` or [`Solo`](https://hackage-content.haskell.org/package/base-4.22.0.0/docs/Data-Tuple.html#t:Solo) `Text` in case the extra indirection implied by `data` still has to happen with -XStrictData that lsp-types uses.

```haskell
data RenameParams = RenameParams 
  { _workDoneToken :: (Maybe Language.LSP.Protocol.Internal.Types.ProgressToken.ProgressToken)
  , _textDocument :: Language.LSP.Protocol.Internal.Types.TextDocumentIdentifier.TextDocumentIdentifier
  , _position :: Language.LSP.Protocol.Internal.Types.Position.Position
  , _newName :: Data.Text.Text
  }
data Position = Position 
  { _line :: Language.LSP.Protocol.Types.Common.UInt
  , _character :: Language.LSP.Protocol.Types.Common.UInt
  }
newtype UInt = UInt (Mod (2 ^ 31))
-- https://hackage-content.haskell.org/package/mod-0.2.1.0/docs/Data-Mod-Word.html#t:Mod
unMod :: Mod m -> Word

data TextDocumentIdentifier = TextDocumentIdentifier 
  { _uri :: Language.LSP.Protocol.Types.Uri.Uri
  }
newtype Uri = Uri {getUri :: Text}
```

Maxima-lsp could have defined

```haskell
data F = F { uri, newName :: Text, line, character :: Int }
f :: RenameParams -> F
f _ = _ -- boilerplate

-- comparable to [json| qq version
rename :: RenameParams -> _
rename (f -> F {uri, newName, line, character}) = _
```

It seems increasingly common experience: the library promises to help, but by the time I understand what the preconditions are, which parts I actually need, I could have done it without the library.

Maxima-lsp is by no means finished but it already helps. There is a cost to picking the wrong variable name. It could be too long and hides the structure or it could be too short and then fails to make a necessary distinction. It can fail to describe how it's made, who needs it or what it's for, and often the purpose is unknown or changes.

The next post should be about using maxima-lsp for simulation or design of a 3D linkage.
