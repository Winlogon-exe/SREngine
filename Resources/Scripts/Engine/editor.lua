local cameraSize;    -- Vector2
local colored_id;    -- int
local final_id;      -- int
local bloom_id;      -- int
local bloomBlur_id;  -- int
local skybox_id;     -- int

local scene;         -- Scene*

local enabled;       -- Bool

function Inspector()

end;

function Hierarchy()
    GUIWindow.Begin("Hierarchy");

    GUIWindow.DrawHierarchy(scene);

    GUIWindow.End();
end;

function Enabled()
    enabled = Stack.PopBool(Script.this);
end;

function SetIndices()
    local camera  = Stack.PopCamera(Script.this);

    scene         = Stack.PopScene(Script.this);

    cameraSize    = camera:GetSize();
    final_id      = camera:GetPostProcessing():GetFinalTextureID();
    colored_id    = camera:GetPostProcessing():GetColoredImage();
    bloom_id      = camera:GetPostProcessing():GetBloomMask();
    bloomBlur_id  = camera:GetPostProcessing():GetBlurBloomMask();
    skybox_id     = 3;--Stack.PopInt(Script.this);

    enabled = true;
end;

function Init()
    Debug.Log("Initialize engine editor GUI...");

    Script.this:ImportLib("Math");
    Script.this:ImportLib("Engine");
    Script.this:ImportLib("Graphics");
    Script.this:ImportLib("GUI");

    collectgarbage() -- collect memory
end;

function Displayes()
    GUIWindow.Begin("Scene");
    GUIWindow.BeginChild("Render");
    GUIWindow.DrawTexture(
        GUIWindow.GetSize(),
        cameraSize,
        final_id,
        true
    );
    GUIWindow.EndChild();
    GUIWindow.End();

    GUIWindow.Begin("Bloom Mask");
    GUIWindow.BeginChild("Render");
    GUIWindow.DrawTexture(
        GUIWindow.GetSize(),
        cameraSize,
        bloom_id,
        true
    );
    GUIWindow.EndChild();
    GUIWindow.End();

    GUIWindow.Begin("Skybox");
    GUIWindow.BeginChild("Render");
    GUIWindow.DrawTexture(
        GUIWindow.GetSize(),
        cameraSize,
        skybox_id,
        true
    );
    GUIWindow.EndChild();
    GUIWindow.End();

    GUIWindow.Begin("Blur Bloom Mask");
    GUIWindow.BeginChild("Render");
    GUIWindow.DrawTexture(
        GUIWindow.GetSize(),
        cameraSize,
        bloomBlur_id,
        true
    );
    GUIWindow.EndChild();
    GUIWindow.End();

    GUIWindow.Begin("Colored");
    GUIWindow.BeginChild("Render");
    GUIWindow.DrawTexture(
        GUIWindow.GetSize(),
        cameraSize,
        colored_id,
        true
    );
    GUIWindow.EndChild();
    GUIWindow.End();
end;

function Windows()
    GUIWindow.Begin("Resources");

    GUIWindow.End();
end;

function ToolBar()
    if (GUIWindow.BeginMainMenuBar()) then
        if (GUIWindow.BeginMenu("File")) then
            if (GUIWindow.MenuItem("New scene")) then

            end;

            if (GUIWindow.MenuItem("Load scene")) then

            end;

            if (GUIWindow.MenuItem("Save scene")) then

            end;

            if (GUIWindow.MenuItem("Exit")) then
                EventManager.Push(Event.Exit);
            end;

            GUIWindow.EndMenu();
        end;

        if (GUIWindow.BeginMenu("Config")) then

            GUIWindow.EndMenu();
        end;

        if (GUIWindow.BeginMenu("Edit")) then

            GUIWindow.EndMenu();
        end;

        if (GUIWindow.BeginMenu("GameObject")) then

            GUIWindow.EndMenu();
        end;

        GUIWindow.EndMainMenuBar();
    end;
end;

function Draw()
    if (enabled == true) then
        DockSpace.Begin();
        Hierarchy();
        Displayes();
        ToolBar();
        Windows();
    end;

    collectgarbage() -- collect memory
end;

function Close()
    Debug.Log("Close engine editor GUI...");
end;