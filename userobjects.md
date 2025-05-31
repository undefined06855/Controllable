# Controllable User Objects

User objects are used to quickly find nodes in a node tree, however any actions
done for specific node types are always checked with a typeinfo cast, so
although types will be assumed, it will likely not crash if an unrelated node
is given a special user object.

You can give your custom nodes these user objects if Controllable doesn't, or
you want them to be focusable or to act as a button, but most likely it will
not do anything and make a bunch of warnings pop up in the console.

It is recommended to set `force-shadowed-selection` and `force-legacy-selection`
on sprites and nodes that need it, as those are guaranteed to work, and will help!

Note that all of these are prefixed with the mod id,
`undefined06855.Controllable/`.

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
    - Set on `DialogLayer`

- `skip-offscreen-checks`
    - Marks that the layer will most likely start offscreen, and that when
    gathering buttons on the layer, to immediately skip to no offscreen check
    mode, instead of seeing if the layer has no found buttons first
    - Set on `GJDropDownLayer`

- `is-special-and-important-notification-node`
    - Marks that the node to be set for CCDirector notification node is from
    this mod, and should be allowed to be set
    - Set on any notification nodes this mod uses, but please **don't** set this
    on your notification nodes as it will **break both mods**!

- `force-shadowed-selection`
    - Marks that the **parent** of this sprite (which should be a `CCMenuItem`),
    when the outline is drawn, to force it to include shadow (likely because
    this sprite is or contains a low opacity sprite)
    - Set on `GeodeTabSprite` and disabled `GJItemIcon`s, but shadowed selection
    is hardcoded on `SliderThumbs`

- `force-legacy-selection`
    - Marks that the **parent** of this sprite (which should be a `CCMenuItem`),
    when the outline is drawn, to force it to be the legacy outline (likely
    because this sprite is either invisible or a shader outline looks ugly).
    This will only apply when the current selection is set to Shader, and will
    not affect the Hover selection type
    - Not set on any nodes by default, but legacy selection is hardcoded on
    `CCTextInputNode`s, because the `CCScale9Sprite` background is not linked to
    the node in any way, as well as the Chamber of Time door when it is unlocked
    but not revealed.
