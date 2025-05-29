# Controllable User Objects

User objects are used to quickly find nodes in a node tree, however any actions
done for specific node types are always checked with a typeinfo cast, so
although types will be assumed, it will likely not crash if an unrelated node
is given a special user object.

- `is-focusable`
    - Marks that the node is focusable
    - Set on `CCMenuItem`, `CCTextInputNode` and `DialogLayer`

- `is-button`
    - Marks that the node should be treated as a button and will be assumed to 
    be able to be casted to `CCMenuItem` (since `activate`, `selected` and
    `unselected` will be called on it)
    - Set on `CCMenuItem`

- `is-text-input`
    - Marks that the node should be treated as a text input, and will be assumed
    to be able to be casted to `CCTextInputNode` (though interactions happen
    through simulated touch, it still gets casted to call the virtual)
    - Set on `CCTextInputNode`

- `is-dialog-layer`
    - Marks that the node should be treated as a dialog layer, and will be
    assumed to be able to be casted to `DialogLayer` (since `handleDialogTap`
    will be called on it)

- `skip-offscreen-checks`
    - Marks that the layer will most likely start offscreen, and that when
    gathering buttons on the layer, to immediately skip to no offscreen check
    mode, instead of seeing if the layer has no found buttons first
    - Set on `GJDropDownLayer`

- `is-special-and-important`
    - Marks that the node to be set for CCDirector notification node is from
    this mod, and should be allowed to be set
    - Please don't set this on your notification nodes as it will break both of
    our mods thanks
