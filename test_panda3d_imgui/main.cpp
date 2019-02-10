/**
 * MIT License
 *
 * Copyright (c) 2019 Younguk Kim
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

#include <pandaFramework.h>
#include <pandaSystem.h>
#include <buttonThrower.h>
#include <mouseWatcher.h>
#include <pgTop.h>

#include <imgui.h>

#include <panda3d_imgui.hpp>

void setup_render(Panda3DImGui* panda3d_imgui_helper)
{
    auto task_mgr = AsyncTaskManager::get_global_ptr();

    // NOTE: ig_loop has process_events and 50 sort.
    PT(GenericAsyncTask) new_frame_imgui_task = new GenericAsyncTask("new_frame_imgui", [](GenericAsyncTask*, void* user_data) {
        static_cast<Panda3DImGui*>(user_data)->new_frame_imgui();
        return AsyncTask::DS_cont;
    }, panda3d_imgui_helper);
    new_frame_imgui_task->set_sort(0);
    task_mgr->add(new_frame_imgui_task);

    PT(GenericAsyncTask) render_imgui_task = new GenericAsyncTask("render_imgui", [](GenericAsyncTask*, void* user_data) {
        static_cast<Panda3DImGui*>(user_data)->render_imgui();
        return AsyncTask::DS_cont;
    }, panda3d_imgui_helper);
    render_imgui_task->set_sort(40);
    task_mgr->add(render_imgui_task);
}

void setup_button(WindowFramework* window_framework, Panda3DImGui* panda3d_imgui_helper)
{
    if (auto bt = window_framework->get_mouse().find("kb-events"))
    {
        auto ev_handler = EventHandler::get_global_event_handler();

        ButtonThrower* bt_node = DCAST(ButtonThrower, bt.node());
        std::string ev_name;

        ev_name = bt_node->get_button_down_event();
        if (ev_name.empty())
        {
            ev_name = "imgui-button-down";
            bt_node->set_button_down_event(ev_name);
        }
        ev_handler->add_hook(ev_name, [](const Event* ev, void* user_data) {
            const auto& key_name = ev->get_parameter(0).get_string_value();
            const auto& button = ButtonRegistry::ptr()->get_button(key_name);
            static_cast<Panda3DImGui*>(user_data)->on_button_down_or_up(button, true);
        }, panda3d_imgui_helper);

        ev_name = bt_node->get_button_up_event();
        if (ev_name.empty())
        {
            ev_name = "imgui-button-up";
            bt_node->set_button_up_event(ev_name);
        }
        ev_handler->add_hook(ev_name, [](const Event* ev, void* user_data) {
            const auto& key_name = ev->get_parameter(0).get_string_value();
            const auto& button = ButtonRegistry::ptr()->get_button(key_name);
            static_cast<Panda3DImGui*>(user_data)->on_button_down_or_up(button, false);
        }, panda3d_imgui_helper);

        ev_name = bt_node->get_keystroke_event();
        if (ev_name.empty())
        {
            ev_name = "imgui-keystroke";
            bt_node->set_keystroke_event(ev_name);
        }
        ev_handler->add_hook(ev_name, [](const Event* ev, void* user_data) {
            wchar_t keycode = ev->get_parameter(0).get_wstring_value()[0];
            static_cast<Panda3DImGui*>(user_data)->on_keystroke(keycode);
        }, panda3d_imgui_helper);
    }
}

void setup_mouse(WindowFramework* window_framework)
{
    window_framework->enable_keyboard();
    auto mouse_watcher = window_framework->get_mouse();
    auto mouse_watcher_node = DCAST(MouseWatcher, mouse_watcher.node());
    DCAST(PGTop, window_framework->get_pixel_2d().node())->set_mouse_watcher(mouse_watcher_node);
}

void on_imgui_new_frame()
{
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static LVecBase3f clear_color = LVecBase3f(0);

    // 1. Show a simple window.
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
    {
        static float f = 0.0f;
        static int counter = 0;
        ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
        ImGui::ColorEdit3("clear color", (float*)&clear_color[0]); // Edit 3 floats representing a color

        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

    // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
    if (show_demo_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
        ImGui::ShowDemoWindow(&show_demo_window);
    }
}

int main()
{
    // open a new window framework
    PandaFramework framework;
    framework.open_framework();

    // set the window title to My Panda3D Window
    framework.set_window_title("My Panda3D Window");

    // open the window
    WindowFramework* window_framework = framework.open_window();
    GraphicsWindow* window = window_framework->get_graphics_window();

    // setup Panda3D mouse for pixel2d
    setup_mouse(window_framework);

    // setup ImGUI for Panda3D
    Panda3DImGui panda3d_imgui_helper(window, window_framework->get_pixel_2d());
    panda3d_imgui_helper.setup_style();
    panda3d_imgui_helper.setup_geom();
    panda3d_imgui_helper.setup_shader(Filename("shader"));
    panda3d_imgui_helper.setup_font();
    panda3d_imgui_helper.setup_event();
    panda3d_imgui_helper.on_window_resized();
    panda3d_imgui_helper.enable_file_drop();

    // setup Panda3D task and key event
    setup_render(&panda3d_imgui_helper);
    setup_button(window_framework, &panda3d_imgui_helper);

    EventHandler::get_global_event_handler()->add_hook("window-event", [](const Event*, void* user_data) {
        static_cast<Panda3DImGui*>(user_data)->on_window_resized();
    }, &panda3d_imgui_helper);

    // use if the context is in different DLL.
    //ImGui::SetCurrentContext(panda3d_imgui_helper.get_context());

    EventHandler::get_global_event_handler()->add_hook(Panda3DImGui::NEW_FRAME_EVENT_NAME, [](const Event*) {
        // draw my GUI
        on_imgui_new_frame();
    });

    // do the main loop, equal to run() in python
    framework.main_loop();

    // close the window framework
    framework.close_framework();

    return 0;
}
