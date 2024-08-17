﻿/*
    Egaroucid Project

    @file button.hpp
        Button for Egaroucid's GUI
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <Siv3D.hpp>
#include <iostream>

struct Button_click{
    bool clicked_down;
    bool pressed;
    bool f_pressed;

    void init(){
        clicked_down = false;
        pressed = false;
        f_pressed = false;
    }

    void update(Rect rect){
        f_pressed = pressed;
        pressed = rect.leftPressed();
        if (rect.leftClicked()){
            clicked_down = true;
        } else if (!MouseL.down() && !f_pressed){
            clicked_down = false;
        }
    }

    void update(RoundRect rect){
        f_pressed = pressed;
        pressed = rect.leftPressed();
        if (rect.leftClicked()){
            clicked_down = true;
        } else if (!MouseL.down() && !f_pressed){
            clicked_down = false;
        }
    }

    bool clicked(){
        return clicked_down && f_pressed && !MouseL.pressed();
    }
};

class Button {
public:
    RoundRect rect;
    String str;
    Font font;
    int font_size;
    Color button_color;
    Color font_color;
private:
    bool enabled;
    Button_click button_click;

public:
    void init(int x, int y, int w, int h, int r, String s, int fs, Font f, Color c1, Color c2) {
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        rect.r = r;
        str = s;
        font_size = fs;
        font = f;
        button_color = c1;
        font_color = c2;
        enabled = true;
        button_click.init();
    }

    void draw() {
        if (enabled){
            rect.draw(button_color);
            font(str).drawAt(font_size, rect.x + rect.w / 2, rect.y + rect.h / 2, font_color);
            if (rect.mouseOver())
                Cursor::RequestStyle(CursorStyle::Hand);
        } else{
            rect.draw(ColorF(button_color, 0.7));
            font(str).drawAt(font_size, rect.x + rect.w / 2, rect.y + rect.h / 2, font_color);
        }
        button_click.update(rect);
    }

    void draw(double transparency) {
        rect.draw(ColorF(button_color, transparency));
        font(str).drawAt(font_size, rect.x + rect.w / 2, rect.y + rect.h / 2, ColorF(font_color, transparency));
        if (rect.mouseOver())
            Cursor::RequestStyle(CursorStyle::Hand);
    }

    bool clicked() {
        return enabled && button_click.clicked();
    }

    void enable(){
        enabled = true;
    }

    void disable(){
        enabled = false;
    }
};

class FrameButton {
public:
    RoundRect rect;
    String str;
    Font font;
    int font_size;
    Color button_color;
    Color font_color;
    Color frame_color;
    int frame_width;
public:
    void init(int x, int y, int w, int h, int r, int fw, String s, int fs, Font f, Color c1, Color c2, Color c3) {
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        rect.r = r;
        frame_width = fw;
        str = s;
        font_size = fs;
        font = f;
        button_color = c1;
        font_color = c2;
        frame_color = c3;
    }

    void draw() {
        rect.draw(button_color).drawFrame(frame_width, frame_color);
        font(str).drawAt(font_size, rect.x + rect.w / 2, rect.y + rect.h / 2, font_color);
        if (rect.mouseOver())
            Cursor::RequestStyle(CursorStyle::Hand);
    }

    void draw(double transparency) {
        rect.draw(ColorF(button_color, transparency)).drawFrame(frame_width, ColorF(frame_color, transparency));
        font(str).drawAt(font_size, rect.x + rect.w / 2, rect.y + rect.h / 2, ColorF(font_color, transparency));
        if (rect.mouseOver())
            Cursor::RequestStyle(CursorStyle::Hand);
    }

    bool clicked() {
        return rect.leftClicked();
    }
};