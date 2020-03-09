#!/usr/bin/env python

"""
File name:  DMA_2_3.py
Author:     Gerbrand De Laender, Damon Verbeyst
Date:       09/03/2020
Email:      gerbrand.delaender@ugent.be, damon.verbeyst@ugent.be
Brief:      E017920A, Design of Multimedia Applications, Assignment
About:      Visualisation of GStreamer video mixing capabilities.
"""

################################################################################
################################################################################

import sys, gi
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
gi.require_version('GdkX11', '3.0')
gi.require_version('GstVideo', '1.0')
from gi.repository import Gst, Gtk, GLib, GdkX11, GstVideo

################################################################################
################################################################################

class VideoMixer():
    
    ############################################################################
    
    def __init__(self):
        """
        A VideoMixer object displays ans stores a mix of two video streams in
        which the opacity of each stream can be controlled using a slider.
        Additional effects can also be added, such as a video overlay.
        """
        Gst.init(None)
        Gtk.init(None)
        self.create_elements()
        self.build_pipeline()
        self.build_ui()
        self.configure_pipeline()
        self.start()
    
    ############################################################################
    
    def create_elements(self):
        """
        Create all GStreamer elements.
        """
        self.EFFECT_NAMES = ("bulge", "circle", "diffuse", "fisheye",
                            "kaleidoscope", "marble", "mirror", "perspective", 
                            "pinch", "rotate", "sphere", "square", "stretch", 
                            "tunnel", "twirl", "waterripple", "burn",
                            "chromium", "dilate", "dodge", "exclusion", 
                            "solarize", "agingtv", "dicetv", "edgetv", "optv",
                            "quarktv", "radioactv", "revtv", "rippletv",
                            "shagadelictv", "streaktv", "vertigotv", "warptv")
                             
        self.src_0 = Gst.ElementFactory.make("uridecodebin", "src_0")
        self.src_1 = Gst.ElementFactory.make("uridecodebin", "src_1")
        self.scl_0 = Gst.ElementFactory.make("videoscale", "scl_0")
        self.scl_1 = Gst.ElementFactory.make("videoscale", "scl_1")
        self.mix_0 = Gst.ElementFactory.make("videomixer", "mix_0")
        self.pix_0 = Gst.ElementFactory.make("gdkpixbufoverlay", "pix_0")
        self.tee_0 = Gst.ElementFactory.make("tee", "tee_0")
        self.que_0 = Gst.ElementFactory.make("queue", "que_0")
        self.que_1 = Gst.ElementFactory.make("queue", "que_1")
        self.vco_0 = Gst.ElementFactory.make("videoconvert", "vco_0")
        self.vco_1 = Gst.ElementFactory.make("videoconvert", "vco_1")
        self.enc_0 = Gst.ElementFactory.make("x264enc", "enc_0")
        self.mux_0 = Gst.ElementFactory.make("matroskamux", "mux_0")
        self.snk_0 = Gst.ElementFactory.make("glimagesink", "snk_0")
        self.snk_1 = Gst.ElementFactory.make("filesink", "snk_1")
        self.pip_0 = Gst.Pipeline.new("pip_0")
        self.bus_0 = self.pip_0.get_bus()
        
        self.effects = [Gst.ElementFactory.make(name, "eff_{}".format(i))
        for i, name in enumerate(self.EFFECT_NAMES)]
        
        if None in (self.src_0, self.src_1, self.scl_0, self.scl_1, self.mix_0,
                    self.pix_0, self.tee_0, self.que_0, self.que_1, self.vco_0, 
                    self.vco_1, self.enc_0, self.mux_0, self.snk_0, self.snk_1,
                    self.pip_0) + tuple(self.effects):
            print("ERROR : Unable to create all elements!")
            sys.exit(1)
        
    ############################################################################
    
    def build_pipeline(self):
        """
        Link all GStreamer elements. The source elements are not yet linked and
        will be linked dynamically.
        """
        for element in (self.src_0, self.src_1, self.scl_0, self.scl_1,
                       self.mix_0, self.pix_0, self.tee_0, self.que_0, 
                       self.que_1, self.vco_0, self.vco_1, self.enc_0,
                       self.mux_0, self.snk_0, self.snk_1) + tuple(self.effects):
           self.pip_0.add(element)
                       
        # Regular linking.
        ret = self.mix_0.link(self.effects[0])
        for i in range(len(self.effects) - 1):
            ret = ret and self.effects[i].link(self.effects[i + 1])
        ret = ret and self.effects[-1].link(self.pix_0)
        ret = ret and self.pix_0.link(self.tee_0)
        ret = ret and self.que_0.link(self.vco_0)
        ret = ret and self.vco_0.link(self.snk_0)
        ret = ret and self.que_1.link(self.vco_1)
        ret = ret and self.vco_1.link(self.enc_0)
        ret = ret and self.enc_0.link(self.mux_0)
        ret = ret and self.mux_0.link(self.snk_1)
        
        if not ret:
            print("ERROR : Unable to link some elements!")
            sys.exit(1)

        # Pad-based linking.
        tmp_0 = self.mix_0.get_pad_template("sink_%u")
        self.pad_0 = self.mix_0.request_pad(tmp_0, None, None)
        ret_0 = self.scl_0.get_static_pad("src").link(self.pad_0)
        tmp_1 = self.mix_0.get_pad_template("sink_%u")
        self.pad_1 = self.mix_0.request_pad(tmp_1, None, None)
        ret_1 = self.scl_1.get_static_pad("src").link(self.pad_1)
        tmp_2 = self.tee_0.get_pad_template("src_%u")
        self.pad_2 = self.tee_0.request_pad(tmp_2, None, None)
        ret_2 = self.pad_2.link(self.que_0.get_static_pad("sink"))
        tmp_3 = self.tee_0.get_pad_template("src_%u")
        self.pad_3 = self.tee_0.request_pad(tmp_3, None, None)
        ret_3 = self.pad_3.link(self.que_1.get_static_pad("sink"))
        
        if any(ret != Gst.PadLinkReturn.OK for ret in (ret_0, ret_1, ret_2,
                                                       ret_3)):
            print("ERROR : Unable to link some pads!")
            sys.exit(1)
    
    ############################################################################
    
    def configure_pipeline(self):
        """
        Configure all GStreamer elements. The input and output file locations
        can be altered from here.
        """
        URI_0 = "file:///home/dma/Downloads/sintel_SD.mp4"
        URI_1 = "file:///home/dma/Downloads/sita_SD.mp4"
        
        self.src_0.set_property("uri", URI_0)
        self.src_0.connect("pad-added", self.on_src_pad_added)
        self.src_1.set_property("uri", URI_1)
        self.src_1.connect("pad-added", self.on_src_pad_added)
        self.pad_0.set_property("alpha", 0.5)
        self.pad_1.set_property("alpha", 0.5)
        self.pix_0.set_property("location", "logo.png")
        self.pix_0.set_property("offset-x", 20)
        self.pix_0.set_property("offset-y", 20)
        self.enc_0.set_property("tune", "zerolatency")
        self.snk_1.set_property("location", "DMA_2_3.mkv")
        
        for eff in self.effects:
            eff.set_passthrough(True)
            
        self.bus_0.add_signal_watch()
        self.bus_0.enable_sync_message_emission()
        self.bus_0.connect("message", self.on_message)
        self.bus_0.connect("sync-message::element", self.on_sync_message)   
        
    ############################################################################
    
    def build_ui(self):
        """
        Build a simple user interface using Gtk in which the video and two
        opacity sliders are shown.
        """
        self.main_window = Gtk.Window.new(Gtk.WindowType.TOPLEVEL)
        self.main_window.connect("delete-event", Gtk.main_quit)

        self.video_window = Gtk.DrawingArea.new()
        
        label_1 = Gtk.Label.new("Alpha 1")
        adj_1 = Gtk.Adjustment.new(50, 0, 100, 0, 0, 0)
        self.slider_1 = Gtk.Scale.new(Gtk.Orientation.HORIZONTAL, None)
        self.slider_1.set_adjustment(adj_1)
        self.slider_1.set_draw_value(False)
        self.slider_1.connect("value-changed", self.on_slider_changed)
        
        label_2 = Gtk.Label.new("Alpha 2")
        adj_2 = Gtk.Adjustment.new(50, 0, 100, 0, 0, 0)
        self.slider_2 = Gtk.Scale.new(Gtk.Orientation.HORIZONTAL, None)
        self.slider_2.set_adjustment(adj_2)
        self.slider_2.set_draw_value(False)
        self.slider_2.connect("value-changed", self.on_slider_changed)
        
        label_3 = Gtk.Label.new("Overlay")
        self.check_3 = Gtk.CheckButton.new_with_label("Enabled")
        self.check_3.set_active(True)
        self.check_3.connect("toggled", self.on_checkbox_toggled, None)
        
        label_4 = Gtk.Label.new("Effect(s)")
        self.scroll_4 = Gtk.ScrolledWindow.new()
        self.scroll_4.set_min_content_height(150)
        self.list_4 = Gtk.ListBox.new()
        self.scroll_4.add(self.list_4)        
        for i, effect_name in enumerate(self.EFFECT_NAMES):
            row = Gtk.ListBoxRow()
            check = Gtk.CheckButton.new_with_label(effect_name)
            check.connect("toggled", self.on_checkbox_toggled, i)
            row.add(check)
            self.list_4.add(row)
    
        box_0 = Gtk.Box(orientation = Gtk.Orientation.VERTICAL, spacing = 0)
        box_1 = Gtk.Box(orientation = Gtk.Orientation.HORIZONTAL, spacing = 0)
        box_2 = Gtk.Box(orientation = Gtk.Orientation.HORIZONTAL, spacing = 0)
        box_3 = Gtk.Box(orientation = Gtk.Orientation.HORIZONTAL, spacing = 7)
        box_4 = Gtk.Box(orientation = Gtk.Orientation.HORIZONTAL, spacing = 0)
        
        box_0.pack_start(self.video_window, True, True, 0)
        box_0.pack_start(box_1, False, True, 0)
        box_0.pack_start(box_2, False, True, 0)
        box_0.pack_start(box_3, False, True, 0)
        box_0.pack_start(box_4, False, True, 0)
        
        box_1.pack_start(label_1, False, True, 10)
        box_1.pack_start(self.slider_1, True, True, 10)
        
        box_2.pack_start(label_2, False, True, 10)
        box_2.pack_start(self.slider_2, True, True, 10)
        
        box_3.pack_start(label_3, False, True, 10)
        box_3.pack_start(self.check_3, True, True, 10)
        
        box_4.pack_start(label_4, False, True, 10)
        box_4.pack_start(self.scroll_4, True, True, 10)
        
        self.main_window.add(box_0)
        self.main_window.set_default_size(800, 600)
        self.main_window.show_all()
    
    ############################################################################
    
    def start(self):
        """
        Start streaming.
        """
        ret = self.pip_0.set_state(Gst.State.PLAYING)
            
        if ret == Gst.StateChangeReturn.FAILURE:
            print("ERROR : Unable to change to playing state!")
        
        Gtk.main()
        
        self.stop()
    
    ############################################################################
    
    def stop(self):
        """
        Stop streaming.
        """
        if self.pip_0:
            self.pip_0.set_state(Gst.State.NULL)
            self.pip_0 = None
    
    ############################################################################
    
    def on_src_pad_added(self, src, new_pad):
        """
        Dynamically link the source pads to the next pads (videoscale). This is
        necessary since the demuxer cannot produce information until some data
        is received and the container is inspected.
        """
        new_pad_name = new_pad.get_current_caps().get_structure(0).get_name()
        
        if not new_pad_name.startswith("video/x-raw"): return
            
        if src == self.src_0:
            sink_pad = self.scl_0.get_static_pad("sink")
        elif src == self.src_1:
            sink_pad = self.scl_1.get_static_pad("sink")
            
        if sink_pad and not sink_pad.is_linked():
            if new_pad.link(sink_pad) == Gst.PadLinkReturn.OK:
                print("INFO : Succesfully linked '{}'!".format(new_pad_name))
            else:
                print("ERROR : Failed to link '{}'!".format(new_pad_name))
            
    ############################################################################
    
    def on_slider_changed(self, range):
        """
        Update the opacity whenever the slider has changed.
        """
        self.pad_0.set_property("alpha", self.slider_1.get_value() / 100)
        self.pad_1.set_property("alpha", self.slider_2.get_value() / 100)
        
    ############################################################################
        
    def on_checkbox_toggled(self, checkbox, data):
        """
        Enable/disable effects based on the checkbox state. If data is None, the
        overlay will be enabled/disabled.
        """
        if data is None:
            if checkbox.get_active():
                self.pix_0.set_property("alpha", 1)
            else:
                self.pix_0.set_property("alpha", 0)
        else:
            if checkbox.get_active():
                self.effects[data].set_passthrough(False)
            else:
                self.effects[data].set_passthrough(True)
        
    ############################################################################
    
    def on_message(self, bus, msg):
        """
        Handle messages that become published on bus_0.
        """
        if bus != self.bus_0: return
        
        if msg.type == Gst.MessageType.STATE_CHANGED:
            if msg.src == self.pip_0:
                old_state, new_state, _ = msg.parse_state_changed()
                old = Gst.Element.state_get_name(old_state)
                new = Gst.Element.state_get_name(new_state)
                print("INFO : Pipeline changed from {} to {}!".format(old, new))    
        elif msg.type == Gst.MessageType.EOS:
            eos = True
            print("INFO : End of stream reached!")
        elif msg.type == Gst.MessageType.ERROR:
            err, dbg = msg.parse_error()
            print("ERROR : {} {}!".format(msg.src.get_name(), err.message))
            print("DEBUG INFO: {}".format(dbg))
            
    ############################################################################
    
    def on_sync_message(self, bus, msg):
        """
        Handle synchronous messages. Intercepts messages such that the window
        handler is set to the window handler of the Gtk video_window instead
        of the default one.
        """
        if msg.get_structure().get_name() == "prepare-window-handle":
            msg.src.set_window_handle(self.video_window.get_window().get_xid())
            
################################################################################

################################################################################
if __name__ == "__main__":
    vm = VideoMixer()
