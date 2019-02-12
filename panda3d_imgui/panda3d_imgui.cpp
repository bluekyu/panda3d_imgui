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

#include "panda3d_imgui.hpp"

#include <cstring>

#include <imgui.h>

#include <throw_event.h>
#include <nodePathCollection.h>
#include <mouseButton.h>
#include <colorAttrib.h>
#include <colorBlendAttrib.h>
#include <depthTestAttrib.h>
#include <cullFaceAttrib.h>
#include <scissorAttrib.h>
#include <geomNode.h>
#include <geomTriangles.h>
#include <graphicsWindow.h>

#if defined(__WIN32__) || defined(_WIN32)
#include <WinUser.h>
#include <shellapi.h>
#endif

class Panda3DImGui::WindowProc : public GraphicsWindowProc
{
public:
    WindowProc(Panda3DImGui& p3d_imgui) : p3d_imgui_(p3d_imgui)
    {
    }

#if defined(__WIN32__) || defined(_WIN32)
    LONG wnd_proc(GraphicsWindow* graphicsWindow, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override
    {
        switch (msg)
        {
            case WM_DROPFILES:
            {
                HDROP hdrop = (HDROP)wparam;
                POINT pt;
                DragQueryPoint(hdrop, &pt);
                p3d_imgui_.dropped_point_ = LVecBase2(static_cast<PN_stdfloat>(pt.x), static_cast<PN_stdfloat>(pt.y));

                const UINT file_count = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);

                std::vector<wchar_t> buffer;
                p3d_imgui_.dropped_files_.clear();
                for (UINT k = 0; k < file_count; ++k)
                {
                    UINT buffer_size = DragQueryFileW(hdrop, k, NULL, 0) + 1;       // #char + \0
                    buffer.resize(buffer_size);
                    UINT ret = DragQueryFileW(hdrop, k, buffer.data(), buffer_size);
                    if (ret)
                    {
                        p3d_imgui_.dropped_files_.push_back(Filename::from_os_specific_w(std::wstring(buffer.begin(), buffer.end() - 1)));
                    }
                }

                throw_event(DROPFILES_EVENT_NAME);

                break;
            }

            default:
            {
                break;
            }
        }

        return 0;
    }
#endif

private:
    Panda3DImGui& p3d_imgui_;
};

// ************************************************************************************************

Panda3DImGui::Panda3DImGui(GraphicsWindow* window, NodePath parent): window_(window)
{
    root_ = parent.attach_new_node("imgui-root", 1000);

    context_ = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    // Setup back-end capabilities flags
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
}

Panda3DImGui::~Panda3DImGui()
{
#if defined(__WIN32__) || defined(_WIN32)
    if (enable_file_drop_)
    {
        if (window_.is_valid_pointer())
        {
            window_->remove_window_proc(window_proc_.get());
            window_proc_.reset();
            if (auto handle = window_->get_window_handle())
                DragAcceptFiles((HWND)handle->get_int_handle(), FALSE);
        }
    }
#endif

    ImGui::DestroyContext();
    context_ = nullptr;
}

void Panda3DImGui::setup_style(Style style)
{
    switch (style)
    {
    case Style::dark:
        ImGui::StyleColorsDark();
        break;
    case Style::classic:
        ImGui::StyleColorsClassic();
        break;
    case Style::light:
        ImGui::StyleColorsLight();
        break;
    }
}

void Panda3DImGui::setup_geom()
{
    PT(GeomVertexArrayFormat) array_format = new GeomVertexArrayFormat(
        InternalName::get_vertex(), 4, Geom::NT_stdfloat, Geom::C_point,
        InternalName::get_color(), 1, Geom::NT_packed_dabc, Geom::C_color
    );

    vformat_ = GeomVertexFormat::register_format(new GeomVertexFormat(array_format));

    root_.set_state(RenderState::make(
        ColorAttrib::make_vertex(),
        ColorBlendAttrib::make(ColorBlendAttrib::M_add, ColorBlendAttrib::O_incoming_alpha, ColorBlendAttrib::O_one_minus_incoming_alpha),
        DepthTestAttrib::make(DepthTestAttrib::M_none),
        CullFaceAttrib::make(CullFaceAttrib::M_cull_none)
    ));
}

void Panda3DImGui::setup_shader(const Filename& shader_dir_path)
{
    root_.set_shader(Shader::load(
        Shader::SL_GLSL,
        shader_dir_path / "panda3d_imgui.vert.glsl",
        shader_dir_path / "panda3d_imgui.frag.glsl",
        "",
        "",
        ""));
}

void Panda3DImGui::setup_shader(Shader* shader)
{
    root_.set_shader(shader);
}

void Panda3DImGui::setup_font()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    setup_font_texture();
}

void Panda3DImGui::setup_font(const char* font_filename, float font_size)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(font_filename, font_size);
    setup_font_texture();
}

void Panda3DImGui::setup_event()
{
    ImGuiIO& io = ImGui::GetIO();

    // for button holder although the variable is not used.
    button_map_ = window_->get_keyboard_map();

    io.KeyMap[ImGuiKey_Tab] = KeyboardButton::tab().get_index();
    io.KeyMap[ImGuiKey_LeftArrow] = KeyboardButton::left().get_index();
    io.KeyMap[ImGuiKey_RightArrow] = KeyboardButton::right().get_index();
    io.KeyMap[ImGuiKey_UpArrow] = KeyboardButton::up().get_index();
    io.KeyMap[ImGuiKey_DownArrow] = KeyboardButton::down().get_index();
    io.KeyMap[ImGuiKey_PageUp] = KeyboardButton::page_up().get_index();
    io.KeyMap[ImGuiKey_PageDown] = KeyboardButton::page_down().get_index();
    io.KeyMap[ImGuiKey_Home] = KeyboardButton::home().get_index();
    io.KeyMap[ImGuiKey_End] = KeyboardButton::end().get_index();
    io.KeyMap[ImGuiKey_Insert] = KeyboardButton::insert().get_index();
    io.KeyMap[ImGuiKey_Delete] = KeyboardButton::del().get_index();
    io.KeyMap[ImGuiKey_Backspace] = KeyboardButton::backspace().get_index();
    io.KeyMap[ImGuiKey_Space] = KeyboardButton::space().get_index();
    io.KeyMap[ImGuiKey_Enter] = KeyboardButton::enter().get_index();
    io.KeyMap[ImGuiKey_Escape] = KeyboardButton::escape().get_index();
    io.KeyMap[ImGuiKey_A] = KeyboardButton::ascii_key('a').get_index();
    io.KeyMap[ImGuiKey_C] = KeyboardButton::ascii_key('c').get_index();
    io.KeyMap[ImGuiKey_V] = KeyboardButton::ascii_key('v').get_index();
    io.KeyMap[ImGuiKey_X] = KeyboardButton::ascii_key('x').get_index();
    io.KeyMap[ImGuiKey_Y] = KeyboardButton::ascii_key('y').get_index();
    io.KeyMap[ImGuiKey_Z] = KeyboardButton::ascii_key('z').get_index();
}

void Panda3DImGui::enable_file_drop()
{
    // register file drop
#if defined(__WIN32__) || defined(_WIN32)
    enable_file_drop_ = true;
    if (window_.is_valid_pointer())
    {
        DragAcceptFiles((HWND)window_->get_window_handle()->get_int_handle(), TRUE);
        window_proc_ = std::make_unique<WindowProc>(*this);
        window_->add_window_proc(window_proc_.get());
    }
#endif
}

void Panda3DImGui::on_window_resized()
{
    if (window_.is_valid_pointer())
        on_window_resized(LVecBase2(static_cast<float>(window_->get_x_size()), static_cast<float>(window_->get_y_size())));
}

void Panda3DImGui::on_window_resized(const LVecBase2& size)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(size[0], size[1]);
    //io.DisplayFramebufferScale;
}

void Panda3DImGui::on_button_down_or_up(const ButtonHandle& button, bool down)
{
    if (button == ButtonHandle::none())
        return;

    ImGuiIO& io = ImGui::GetIO();
    if (MouseButton::is_mouse_button(button))
    {
        if (button == MouseButton::one())
        {
            io.MouseDown[0] = down;
        }
        else if (button == MouseButton::three())
        {
            io.MouseDown[1] = down;
        }
        else if (button == MouseButton::two())
        {
            io.MouseDown[2] = down;
        }
        else if (button == MouseButton::four())
        {
            io.MouseDown[3] = down;
        }
        else if (button == MouseButton::five())
        {
            io.MouseDown[4] = down;
        }
        else if (down)
        {
            if (button == MouseButton::wheel_up())
                io.MouseWheel += 1;
            else if (button == MouseButton::wheel_down())
                io.MouseWheel -= 1;
            else if (button == MouseButton::wheel_right())
                io.MouseWheelH += 1;
            else if (button == MouseButton::wheel_left())
                io.MouseWheelH -= 1;
        }
    }
    else
    {
        io.KeysDown[button.get_index()] = down;

        if (button == KeyboardButton::control())
            io.KeyCtrl = down;
        else if (button == KeyboardButton::shift())
            io.KeyShift = down;
        else if (button == KeyboardButton::alt())
            io.KeyAlt = down;
        else if (button == KeyboardButton::meta())
            io.KeySuper = down;
    }
}

void Panda3DImGui::on_keystroke(wchar_t keycode)
{
    if (keycode < 0 || keycode >= (std::numeric_limits<ImWchar>::max)())
        return;

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(keycode);
}

bool Panda3DImGui::new_frame_imgui()
{
    if (root_.is_hidden())
        return false;

    static const int MOUSE_DEVICE_INDEX = 0;

    ImGuiIO& io = ImGui::GetIO();

    io.DeltaTime = static_cast<float>(ClockObject::get_global_clock()->get_dt());

    if (window_.is_valid_pointer() && window_->is_of_type(GraphicsWindow::get_class_type()))
    {
        const auto& mouse = window_->get_pointer(MOUSE_DEVICE_INDEX);
        if (mouse.get_in_window())
        {
            if (io.WantSetMousePos)
            {
                window_->move_pointer(MOUSE_DEVICE_INDEX, io.MousePos.x, io.MousePos.y);
            }
            else
            {
                io.MousePos.x = static_cast<float>(mouse.get_x());
                io.MousePos.y = static_cast<float>(mouse.get_y());
            }
        }
        else
        {
            io.MousePos.x = -FLT_MAX;
            io.MousePos.y = -FLT_MAX;
        }
    }

    ImGui::NewFrame();

    throw_event_directly(*EventHandler::get_global_event_handler(), NEW_FRAME_EVENT_NAME);

    return true;
}

bool Panda3DImGui::render_imgui()
{
    if (root_.is_hidden())
        return false;

    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    const float fb_width = io.DisplaySize.x * io.DisplayFramebufferScale.x;
    const float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;

    auto draw_data = ImGui::GetDrawData();
    //draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    auto npc = root_.get_children();
    for (int k = 0, k_end = npc.get_num_paths(); k < k_end; ++k)
        npc.get_path(k).detach_node();

    for (int k = 0; k < draw_data->CmdListsCount; ++k)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[k];

        if (!(k < static_cast<int>(geom_data_.size())))
        {
            geom_data_.push_back({
                new GeomVertexData("imgui-vertex-" + std::to_string(k), vformat_, GeomEnums::UsageHint::UH_stream),
                {}
            });
        }

        auto& geom_list = geom_data_[k];

        auto vertex_handle = geom_list.vdata->modify_array_handle(0);
        if (vertex_handle->get_num_rows() < cmd_list->VtxBuffer.Size)
            vertex_handle->unclean_set_num_rows(cmd_list->VtxBuffer.Size);

        std::memcpy(
            vertex_handle->get_write_pointer(),
            reinterpret_cast<const unsigned char*>(cmd_list->VtxBuffer.Data),
            cmd_list->VtxBuffer.Size * sizeof(decltype(cmd_list->VtxBuffer)::value_type));

        auto idx_buffer_data = cmd_list->IdxBuffer.Data;
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
        {
            const ImDrawCmd* draw_cmd = &cmd_list->CmdBuffer[cmd_i];
            auto elem_count = static_cast<int>(draw_cmd->ElemCount);

            if (!(cmd_i < static_cast<int>(geom_list.nodepaths.size())))
                geom_list.nodepaths.push_back(create_geomnode(geom_list.vdata));

            NodePath np = geom_list.nodepaths[cmd_i];
            np.reparent_to(root_);

            auto gn = DCAST(GeomNode, np.node());

            auto index_handle = gn->modify_geom(0)->modify_primitive(0)->modify_vertices(elem_count)->modify_handle();
            if (index_handle->get_num_rows() < elem_count)
                index_handle->unclean_set_num_rows(elem_count);

            std::memcpy(
                index_handle->get_write_pointer(),
                reinterpret_cast<const unsigned char*>(idx_buffer_data),
                elem_count * sizeof(decltype(cmd_list->IdxBuffer)::value_type));
            idx_buffer_data += elem_count;

            CPT(RenderState) state = RenderState::make(ScissorAttrib::make(
                draw_cmd->ClipRect.x / fb_width,
                draw_cmd->ClipRect.z / fb_width,
                1 - draw_cmd->ClipRect.w / fb_height,
                1 - draw_cmd->ClipRect.y / fb_height));

            if (draw_cmd->TextureId)
                state = state->add_attrib(TextureAttrib::make(static_cast<Texture*>(draw_cmd->TextureId)));

            gn->set_geom_state(0, state);
        }
    }

    return true;
}

void Panda3DImGui::setup_font_texture()
{
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    font_texture_ = Texture::make_texture();
    font_texture_->set_name("imgui-font-texture");
    font_texture_->setup_2d_texture(width, height, Texture::ComponentType::T_unsigned_byte, Texture::Format::F_red);
    font_texture_->set_minfilter(SamplerState::FilterType::FT_linear);
    font_texture_->set_magfilter(SamplerState::FilterType::FT_linear);

    PTA_uchar ram_image = font_texture_->make_ram_image();
    std::memcpy(ram_image.p(), pixels, width * height * sizeof(decltype(*pixels)));

    io.Fonts->TexID = font_texture_.p();
}

NodePath Panda3DImGui::create_geomnode(const GeomVertexData* vdata)
{
    PT(GeomTriangles) prim = new GeomTriangles(GeomEnums::UsageHint::UH_stream);

    static_assert(
        sizeof(ImDrawIdx) == sizeof(uint16_t) ||
        sizeof(ImDrawIdx) == sizeof(uint32_t),
        "Type of ImDrawIdx is not uint16_t or uint32_t. Update below code!"
        );
    if (sizeof(ImDrawIdx) == sizeof(uint16_t))
        prim->set_index_type(GeomEnums::NumericType::NT_uint16);
    else if (sizeof(ImDrawIdx) == sizeof(uint32_t))
        prim->set_index_type(GeomEnums::NumericType::NT_uint32);

    prim->close_primitive();

    PT(Geom) geom = new Geom(vdata);
    geom->add_primitive(prim);

    PT(GeomNode) geom_node = new GeomNode("imgui-geom");
    geom_node->add_geom(geom, RenderState::make_empty());

    return NodePath(geom_node);
}
