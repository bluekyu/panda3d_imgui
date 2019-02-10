/**
 * MIT License
 *
 * Copyright (c) 2018-2019 Younguk Kim (bluekyu)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <nodePath.h>

class Texture;
class ButtonMap;
class GraphicsWindow;
class ButtonHandle;

struct ImGuiContext;

class Panda3DImGui
{
public:
    static constexpr const char* NEW_FRAME_EVENT_NAME = "imgui-new-frame";
    static constexpr const char* SETUP_CONTEXT_EVENT_NAME = "imgui-setup-context";
    static constexpr const char* DROPFILES_EVENT_NAME = "imgui-dropfiles";

    enum class Style
    {
        dark = 0,
        classic,
        light,
    };

public:
    Panda3DImGui(GraphicsWindow* window, NodePath parent);
    ~Panda3DImGui();

    void setup_style(Style style = Style::dark);
    void setup_geom();
    void setup_shader(const Filename& shader_dir_path);
    void setup_shader(Shader* shader);
    void setup_font();
    void setup_font(const char* font_filename, float font_size);
    void setup_event();
    void enable_file_drop();

    void on_window_resized();
    void on_window_resized(const LVecBase2& size);
    void on_button_down_or_up(const ButtonHandle& button, bool down);
    void on_keystroke(wchar_t keycode);

    bool new_frame_imgui();
    bool render_imgui();

    ImGuiContext* get_context() const;
    NodePath get_root() const;

    /** Get dropped files. */
    const std::vector<Filename>& get_dropped_files() const;

    /** Get mouse position when files are dropped. */
    const LVecBase2& get_dropped_point() const;

private:
    void setup_font_texture();
    NodePath create_geomnode(const GeomVertexData* vdata);

    ImGuiContext* context_ = nullptr;

    WPT(GraphicsWindow) window_;
    NodePath root_;
    PT(Texture) font_texture_;
    PT(ButtonMap) button_map_;
    CPT(GeomVertexFormat) vformat_;

    struct GeomList
    {
        PT(GeomVertexData) vdata;           // vertex data shared among the below GeomNodes
        std::vector<NodePath> nodepaths;
    };
    std::vector<GeomList> geom_data_;

    class WindowProc;
    std::unique_ptr<WindowProc> window_proc_;
    bool enable_file_drop_ = false;
    std::vector<Filename> dropped_files_;
    LVecBase2 dropped_point_;
};

// ************************************************************************************************

inline ImGuiContext* Panda3DImGui::get_context() const
{
    return context_;
}

inline NodePath Panda3DImGui::get_root() const
{
    return root_;
}

inline const std::vector<Filename>& Panda3DImGui::get_dropped_files() const
{
    return dropped_files_;
}

inline const LVecBase2& Panda3DImGui::get_dropped_point() const
{
    return dropped_point_;
}
