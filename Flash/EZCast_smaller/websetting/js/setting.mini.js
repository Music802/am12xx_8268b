var text_indeed;
var start;
var stop;
var text_string;
var defaultmode_index = -1;
var save_defaultmode_index;
var connection_mode_index = -1;
var current_connection_mode_index = -1;
var show_waiting_tick_id = -1;
var show_waiting_tick = 11;
var appskValue;
var appskValue_save;
var warn_msg;
var default_mode_str;
var defaultmode_global_text;
var function_index;
var show_warn_dialog_tick = 10;
var show_warn_dialog_id = -1;
var check_OTA_first_flag = -1;
var Upgrade_listener_flag = -1;
var client_connect_show_flag = -1;
var connected_ssid_flag = 0;
var show_connections_num_id = -1;
var get_connected_ssid_id = -1;
var screen_width, screen_height, sWidth, sHeight;
var appsk_input_flag = -1;
var upgrade_msg;
var upgrade_connect_fail_OK_CANCEL_flag = -1;
var Check_new_version_boot_id = -1;
var get_ota_check_status_tick = 0;
var get_new_version_timer_id = -1;
var delay_get_data_id = -1;
var get_ota_status_id = -1;
var upgrade_dialog_msg_string;
var upgrade_warn_string;
var PSW_LEN_string = "Password must be between 8 and 15 characters long.";
var PSW_ALLOW_string = "Only the letters and numbers are allowed!";
var LIST_WARN2_string = "Access Point will be disconnected if you press OK.";
var LIST_WARN3_string="Please enter a new password to save again";
var changes_effect_string="Changes take effect after reboot.";
var waiting_string,password_string;
var wifi_mode_status;
var wifi_connect_flag;
var resolution_array = new Array();
var resolution_array = ["1024x768_60P","1280x720_60P", "1280x800_60P","1920x1080_24P", "1920x1080_30P", "1920x1080_60P"];
var resolution_list_value=new Array (3,0,4,1,2,5);
//var resolution_array = ["1280x720_60P", "1920x1080_24P", "1920x1080_30P", "1920x1080_60P"];
//var resolution_list_value=new Array (0,1,2,5);
var resolution_index = 0;
var wifi_mode_flag = 0;
var current_resolution_index = -1;
var current_wlan_ip_value = -1;
var resolution_current_value;
var resolution_current_value_num;
var defaultmode_array = new Array();
var defaultmode_array = ["Airplay", "Miracast"];
var ezairmode_index = 0,ezairmode_select = 0,select_txt,ezarimode_txt="EZAir mode";
var ezairmode_array = new Array();
var ezairmode_array = ["Mirror+Stream","Mirror Only"];
var upgrade_bar = 0;
var ota_auto_flag=0;
var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();

function Get_height_width_of_screen() {

    sWidth = document.documentElement.scrollWidth || document.body.scrollWidth;
    sHeight = document.documentElement.scrollHeight || document.body.scrollHeight;

    if ( !! (window.attachEvent && !window.opera)) {
        screen_height = document.documentElement.clientHeight;
        screen_width = document.documentElement.clientWidth;
    } else {
        screen_height = window.innerHeight;
        screen_width = window.innerWidth;
    }
    //document.getElementById('msg').value  ='width:' + screen_width + '; height:'+screen_height;
}


window.onresize = window.onload = function() {
    Get_height_width_of_screen();
}

function show_wait() {
    $("#waiting").css('display', 'block');
    $("#waitbk").css('display', 'block');
}

function show_ui() {
    $("#waiting").css('display', 'none');
    $("#waitbk").css('display', 'none');
    $("#list_load").css('display', 'block');
	
}


function To_wifilist() {
    location.href = "wifilist.html";
}

/************************************user new copy*************************************/

function Get_ota_status_tick() {

    var cgiurll = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurll, {type: 'get_ota_status'},function(ota_status)
    {
        upgrade_bar = ota_status;
	console.log("upgrade_% :"+upgrade_bar)
        $("#progress_label").text(upgrade_bar + "%");
        $("#progressbar").css("width", upgrade_bar + "%");
		
        if (ota_status >= 100) {
            clearInterval(get_ota_status_id);
            get_ota_status_id = -1;
        }
    },"text");
}
function Get_ota_status() {
    if (get_ota_status_id > 0) {
        clearInterval(get_ota_status_id);
        get_ota_status_id = -1;
    }
    get_ota_status_id = self.setInterval(Get_ota_status_tick, 1000);
}



function Get_upgrade_language() {
            $('#upgrade_dialog_msg').text("Do you want to upgrade?");
            $('#server_text').text("server version :");
            $('#local_text').text("local version :");
            $('#upgrade_warn').text("Access Point will be disconnected if you press OK.");
            upgrade_dialog_msg_string = "Please wait for downloading.";
            upgrade_warn_string = "After download, wifi will be disconnected and please wait for system upgrading and reboot.";
}
function show_warn_dialog_tick_function() {
    if (show_warn_dialog_tick < 1)
    {
        $('#ota_fail').hide();
        $('#warn').hide();
        $('#warn_time').hide();

        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
        show_warn_dialog_tick = 11;
    } else {
        show_warn_dialog_tick = show_warn_dialog_tick - 1;
        $('#upgrade_tick').text("" + show_warn_dialog_tick);
    }
}
function show_OTA_warn_dialog() {
    show_warn_dialog_tick = 10;
    $('#upgrade_tick').text("" + show_warn_dialog_tick);
    //$('#ota_fail').popup('open');
    if (show_warn_dialog_id > 0) {
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
    }
    show_warn_dialog_id = self.setInterval(show_warn_dialog_tick_function, 1000);
}


function show_faild_connect_tick_function() {
    if (show_warn_dialog_tick < 1) 
    {
        $('#ota_fail').hide();
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
        show_warn_dialog_tick = 11;
    } else {
        show_warn_dialog_tick = show_warn_dialog_tick - 1;
        $('#upgrade_tick').text("" + show_warn_dialog_tick);
    }
}


function show_faild_connect_dialog() {
    show_warn_dialog_tick = 10;
    $('#upgrade_tick').text("" + show_warn_dialog_tick);
    $('#ota_fail').show();
    $('#warn').hide();
    $('#warn_time').show();
    if (show_warn_dialog_id > 0) {
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
    }
    show_warn_dialog_id = self.setInterval(show_faild_connect_tick_function, 1000);
    upgrade_connect_fail_OK_CANCEL_flag = 1;
   
}

function Get_faild_connect_language() {
    $('#upgrade_dialog_msg2').text("Faild to connect,please try again.");
    $('#upgrade_time_tick_msg').text("sec left to return your setting last time.");

}

function delaydisp_popup() {
    $('#warn').hide();
    $('#warn_time').hide();
    $('#ota').show();
}

function set_version_string(ota_upgrade_string) {
    if (ota_upgrade_string.length > 0) {
        //if ((check_OTA_first_flag == 1) || (Upgrade_listener_flag == 1)) {
        //}
        var warn_ret = ota_upgrade_string.indexOf("newest");
        if (warn_ret < 0) {
            $('#ota_fail').hide(); 
            if (0 == connected_ssid_flag) {
                var ret = ota_upgrade_string.indexOf("\n");
                var local_version_string = ota_upgrade_string.substring(0, ret);
                $('#new_version').val(local_version_string);
            } else {
                Get_new_version_text();
            }
            if ((check_OTA_first_flag == 1) || (Upgrade_listener_flag == 1)) {
                if (check_OTA_first_flag == 1) 
                    check_OTA_first_flag = -1;
                else if (Upgrade_listener_flag == 1) 
                    Upgrade_listener_flag = -1;
                Get_upgrade_language();
                var ret = ota_upgrade_string.indexOf("\n");
                var local_version_string = ota_upgrade_string.substring(0, ret);
                console.log("local_version: "+local_version_string);
                $('#local_version').val(local_version_string);
                var server_version_string = ota_upgrade_string.substring(ret + 1, ota_upgrade_string.length);
                $('#server_version').val(server_version_string);
                console.log("server_version: "+server_version_string);
                setTimeout(delaydisp_popup, 500);
            }
        } else //show newest 
        {
            if (Upgrade_listener_flag == 1) {
                Upgrade_listener_flag = -1;
                check_OTA_first_flag = -1;
                get_OTA_warn_language(1);
                $('#warn').hide();
                $('#warn_time').show();
                show_OTA_warn_dialog();
                upgrade_connect_fail_OK_CANCEL_flag = 1;
            } else {
                var ret = ota_upgrade_string.indexOf("\n");
                var local_version_string = ota_upgrade_string.substring(0, ret);
                $('#new_version').val(local_version_string);
                $('#ota_fail').hide();
                $('#warn').hide();
                $('#warn_time').hide();

            }
        }
    }

}
/*
function get_new_version_tick_function() {
    if (get_new_version_timer_id > 0) {
        clearInterval(get_new_version_timer_id);
        get_new_version_timer_id = -1;
    }
    $.get(cgiurl, {type: 'get_OTA_version'},function(ota_upgrade_string) {
        if (ota_upgrade_string.length) 
            set_version_string(ota_upgrade_string);
    },"text");
}
*/
function get_new_version_function() {
    $.get(cgiurl, {type: 'get_OTA_version'},function(ota_upgrade_string) {
        if (ota_upgrade_string.length) {
            set_version_string(ota_upgrade_string);
        }
    },"text");
}


function get_OTA_warn_language(flag) {
    if (flag) {
                $('#upgrade_dialog_msg2').text("Your firmware is the latest version.");
                $('#upgrade_time_tick_msg').text("sec left to return your setting last time.");

    } else {
                $('#upgrade_dialog_msg2').text("The server version of firmware is unavailable.");
                $('#upgrade_time_tick_msg').text("sec left to return your setting last time.");
    }
}

function get_ota_check_status_tick_function() {
    $.get(cgiurl, {type: 'get_ota_check_status'},function(new_ver_flag) {
        get_ota_check_status_tick++;
        if (get_ota_check_status_tick > 50) //20
        {
            get_ota_check_status_tick = 0;
            if (Check_new_version_boot_id > 0) {
                clearInterval(Check_new_version_boot_id);
                Check_new_version_boot_id = -1;
            }
            $('#ota_fail').hide();
            $('#warn').hide();
            $('#warn_time').hide();

        } else {
            var ret = Number(new_ver_flag);
            if (1 == ret) //check finish
            {
                get_ota_check_status_tick = 0;
                if (Check_new_version_boot_id > 0) {
                    clearInterval(Check_new_version_boot_id);
                    Check_new_version_boot_id = -1;
                }
                get_new_version_function();
            }
        }
    },"text");
}
function get_ota_check_status() {
    if (Check_new_version_boot_id > 0) {
        clearInterval(Check_new_version_boot_id);
        Check_new_version_boot_id = -1;
    }
    Check_new_version_boot_id = self.setInterval(get_ota_check_status_tick_function, 1000);
}

function get_wifi_mode() {
    $.get(cgiurl, {type: 'get_wifi_mode'},function(wifi_mode) {
		console.log("ota ->get_wifi_mode :"+wifi_mode);
		console.log("ota ->Upgrade_listener_flage :"+Upgrade_listener_flag);
        wifi_mode_status = wifi_mode.indexOf("9");
        if (wifi_mode_status >= 0) {
            if (!wifi_connect_flag) 
            {
                Get_faild_connect_language();
                show_faild_connect_dialog(); //no wifi connect warn
            } else {
                if (Upgrade_listener_flag == 1) {
                    $('#upgrade_dialog_msg').text("Please wait");
                    $('#ota_fail').show();
                    $('#warn').show();
                    $('#warn_time').hide();
                }
                $.get(cgiurl, {type: 'get_OTA_conf'},function() {

                }, "text");
                get_ota_check_status();
            }
        } else {
            Get_faild_connect_language();
            show_faild_connect_dialog(); //no wifi connect warn
        }
    },"text");
}


//ota upgrade button click
function Upgrade_listener() {
    $('#upgrade').click(function(e) {
        var ret;
        e.preventDefault();
        Upgrade_listener_flag = 1;
        get_wifi_mode();
    });
}
function changeota() {
    $("#1").css('display', 'none');
    $("#2").css('display', 'block');
}
//ota upgrade button click
function Upgrade_OK_listener() {
    $('#upgrade_OK').click(function() {
        changeota();
        if (upgrade_connect_fail_OK_CANCEL_flag == 1) {
            if (show_connections_num_id > 0) {
                clearInterval(show_connections_num_id);
                show_connections_num_id = -1;
            }
        } else {
            $.get(cgiurl, {type: 'get_down_bin'},function(return_val) {
                var ret = return_val.indexOf("OK");
                if (ret >= 0) {
                    Get_ota_status();
                }
            },"text");
            if (show_connections_num_id > 0) {
                clearInterval(show_connections_num_id);
                show_connections_num_id = -1;
            }
            $('#upgrade_warn').text(upgrade_warn_string);
        }
    });


    $('#upgrade_CANCEL').click(function() {
        $('#ota').hide();
    });

    $('#ota_awrn_ok').click(function() {
        $('#ota_fail').hide();
        $('#warn').show();
        $('#warn_time').hide();
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
        show_warn_dialog_tick = 11;
    });

}


var loopnum = 0;
var lan_con = 0;
function Get_connected_ssid_tick() {
    loopnum++;
    if (loopnum > 10) {
        clearInterval(get_connected_ssid_id);
        get_connected_ssid_id = -1;
    }
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    $.get(cgiurl, {type: 'get_connected_ssid'},function(connected_ssid) {
        if (!Array.indexOf) {
            Array.prototype.indexOf = function(obj) {
                for (var i = 0; i < this.length; i++) {
                    if (this[i] == obj) {
                        return i;
                    }
                }
                return - 1;
            }
        }
		console.log("Get_connected_ssid_tick: "+connected_ssid);
        var ret = connected_ssid.indexOf("to_wifi_list");
        if (ret >= 0) {
            lan_con = connected_ssid.indexOf("to_wifi_list&lan");
            if (lan_con >= 0) 
                lan_con = 1;
            else 
                $('#connected_ssid').hide();
        } else {
            if (connected_ssid.length && lan_con != 1) {
                if (connected_ssid_flag == 0) {
                    $('#connected_ssid').text(connected_ssid);
                    $('#connected_ssid').show();
                    wifi_connect_flag = 1;
                    connected_ssid_flag = 1;
                    show_connections_num_id = self.setInterval(Get_local_version, 3000);
                }
            } else {
                //$('#connected_ssid').hide();
                $('#connected_ssid').text("no wifi connect!");
                wifi_connect_flag = 0;
                connected_ssid_flag = 0;
            }
        }
    },"text");
}


function Get_new_version_text() {
    $.get(cgiurl, {type: 'get_new_ver_text'},function(new_version_text) {
        if (new_version_text.length) {
            $('#new_version').show();
            $('#new_version').text(new_version_text);
            $('#upg_txt_2').text("--2_New Version!--"); 
        } else {
            $('#new_version').show();
            $('#new_version').text("New Version!");
            $('#upg_txt').text("New Version!");

        }
    },"text");
}

function Get_connected_ssid() {
    Get_connected_ssid_tick();
    if (get_connected_ssid_id > 0) {
        clearInterval(get_connected_ssid_id);
        get_connected_ssid_id = -1;
    }
    loopnum = 0;
    get_connected_ssid_id = self.setInterval(Get_connected_ssid_tick, 3000);
}
function Get_text_from_textstring(text_string) {
    var text_string_tmp = text_string.substring(start, text_string_length);
    stop = text_string_tmp.indexOf("\n");
    start = 0;
    text_indeed = text_string_tmp.substring(start, stop);
    start = stop + 1;
    text_string_global = text_string_tmp;
    return text_indeed;
}

//read current version
var loopnum_currver = 0;
function Get_local_version() {
    loopnum_currver++;
    if (loopnum_currver > 5) {
        clearInterval(show_connections_num_id);
        show_connections_num_id = -1;
    }
    $.get(cgiurl, {type: 'get_OTA_version'},function(ota_upgrade_string) {
		console.log("Get_local_version: "+ ota_upgrade_string);
        if (ota_upgrade_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = ota_upgrade_string;
            text_string_length = ota_upgrade_string.length;
            var local_version_string = Get_text_from_textstring(text_string);
            $('#current_ver').text(local_version_string);
            $('#new_version').text(local_version_string);
            text_string = text_string_global;
            var server_version_string = Get_text_from_textstring(text_string);
            $.get(cgiurl, {type: 'get_wifi_mode'},function(wifi_mode) {
                wifi_mode_status = wifi_mode.indexOf("9");
                if (wifi_mode_status >= 0) {
                    var ret = server_version_string.indexOf("newest");
                    if (ret < 0) {
                        Get_new_version_text();
                        clearInterval(show_connections_num_id);
                        show_connections_num_id = -1;
                        loopnum_currver = 6;
                        clearInterval(get_connected_ssid_id);
                        get_connected_ssid_id = -1;
                        loopnum = 20;
                    } else {
                        $('#new_version').show();
                        $('#new_version').text("no new version!");
                    }
                } else {
                    $('#upg_txt').text("no wifi_mode_status");
                    //$('#new_version').hide();
                    $('#new_version').text("not wifi connect!");
                }
            },"text");
        }
    },"text");
}
function Check_new_version_boot() {
    $.get(cgiurl, {type: 'get_wifi_mode'},function(wifi_mode) {
        wifi_mode_status = wifi_mode.indexOf("9");
        if (wifi_mode_status >= 0) {
            $.get(cgiurl, {type: 'enter_check_ota_boot'},function(return_value) {
                var ret = Number(return_value);
                if (0 == ret) {
                    $.get(cgiurl, {type: 'get_OTA_conf'},function() {

                    }, "text");
                    get_ota_check_status();
                }
            },"text");
        }
    },"text");
}


function Set_hdmimode() {
    $.post('cgi-bin/set_resolution_POST.cgi', $('#config-form').serialize());
}

function resolution_set_default_value() {
    var i=0;
    $('#res_value').val("" + resolution_current_value_num);
    $('#current_res_value').val("" + 0);
    resolution_index = current_resolution_index;
    $('#resolution_sel').text(resolution_array[resolution_index]);
    resolution_current_value = $('#resolution_sel').text();
}
function get_mirascreen5g_flag(){
    var cgiurll = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurll, {type: 'get_mirascreen5g_flag'},function(flag)
    {
        if (Number(flag)==1) {
			
		$("#resolution_all").remove();
        }
   	 else
	  	get_ddr_type();
    },"text");
}
function get_ddr_type() {
    var cgiurll = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurll, {type: 'read_ddrtype'},function(type)
    {
        $("#res_0_txt,#res_0,#res_2_txt,#res_2").remove();
        if (Number(type)==2) {
			
             $("#res_5_txt,#res_5").remove();
        }
   	 set_resolutionradio();
    },"text");
}

function resolution_timetick_function() {
    $('#resolution_tick').html(show_waiting_tick);
    if (show_waiting_tick == 0) {
        if (show_waiting_tick_id > 0) {
            clearInterval(show_waiting_tick_id);
            show_waiting_tick_id = -1;
            resolution_set_default_value();
            //alert("--resolution_timetick--");
            Set_hdmimode();
            show_waiting_tick = 11;
            $("#Resolution").hide();
        }
    } else show_waiting_tick = show_waiting_tick - 1;

}
function set_resolution_timetick() {
    if (show_waiting_tick_id > 0) {
        clearInterval(show_waiting_tick_id);
        show_waiting_tick_id = -1;
    }
    show_waiting_tick = 15;
    show_waiting_tick_id = self.setInterval(resolution_timetick_function, 1000);
}
//get resolution text for ui
function Get_resolution_text() {
        $('#dialog_msg').text("Use the new Resolution ?");
        $('#time_tick_msg').text("sec left to return your setting last time.");
        $('#reboot_msg').text("System will reboot if you choose OK!");
        $('#dialog_msg2').text("Please switch to other resolutions if the display is choppy when streaming high bit rate videos for 1920x1080 60P.");
}
//resolution button
function Set_resolution_listener() 
{
    $("#set_resolution_ok").click(function() {
        Get_resolution_text();
        if (resolution_index != current_resolution_index) {
                $('#dialog_msg2').hide();
    		
            $("#res_sel").css('display', 'none');
            $("#res_warn").css('display', 'block');
            $('#resolution_tick').html("15");//time cnt
            $('#current_res_value').val("" + resolution_current_value_num);
            Set_hdmimode();
            if (((1 == current_resolution_index) && (2 == resolution_index)) || ((2 == current_resolution_index) && (1 == resolution_index))) 
                $('#reboot_msg').hide();
            else if (((17 == current_resolution_index) && (2 == resolution_index)) || ((2 == current_resolution_index) && (17 == resolution_index))) 
                $('#reboot_msg').hide();
            else 
                $('#reboot_msg').show();
            set_resolution_timetick();
        }
    });


    $("#resolution_ent").click(function() {
        var i=0;
        clearInterval(show_waiting_tick_id);
        show_waiting_tick_id = -1;
        var temp = $('#res_value').val();
        resolution_current_value_num = $('#res_value').val();
        temp = temp + "OK";
        $('#res_value').val("" + temp);


            for(i=0;i<resolution_list_value.length;i++){
                if(resolution_list_value[i]==resolution_index)
                    break;
            }
            $('#resolution_sel').text(resolution_array[i]);

		
        resolution_current_value = $('#resolution_sel').text();
        current_resolution_index=resolution_index;
        Set_hdmimode();
        $("#Resolution").hide();
    });

    $("#resolution_cancle").click(function() {
		clearInterval(show_waiting_tick_id);
		show_waiting_tick_id = -1;
		resolution_set_default_value();
		Set_hdmimode();
		$("#Resolution").hide();
    });

    $("#set_resolution_cal").click(function() {
		resolution_index = current_resolution_index;
		$('#Resolution').hide();

    });

    $('input:radio[name="resolution_choose"]').click(function() {
        var that = $(this).val();
        var b = $(this).prev().text();
        resolution_index = Number(that);
        $('#res_text').text(resolution_index);
        switch (resolution_index) {
            case 0:
                $('#res_value').val("5"); //FMT_1650x750_1280x720_60P
                break;
            case 1:
                $('#res_value').val("4"); //FMT_2750x1125_1920x1080_24P
                break;
            case 2:
                $('#res_value').val("2"); //FMT_2200x1125_1920x1080_30P
                break;
            case 3:
                $('#res_value').val("15"); //FMT_1250x810_1024x768_60P
                break;
            case 4:
                $('#res_value').val("16"); //FMT_1280x800_60P
                break;
            case 5:
                $('#res_value').val("17"); //FMT_1920x1080_60P
                break;
        }
          console.log("select   "+resolution_index);

    });

    $('#resolution_all').click(function() {
		console.log("resolution_all  click"+resolution_index);
        $('input:radio[name="resolution_choose"]').each(function() {
			console.log("each radio: "+$(this).val());
			// $(this).removeAttr("checked");
			$(this).attr("checked", " ");
        });
        set_resolutionradio();
        $('#Resolution').show();
        $("#res_sel").css('display', 'block');
        $("#res_warn").css('display', 'none');
    });

}
//set resolution css
function set_resolutionradio() {
    var that = $("input[type=radio][name=resolution_choose][value=" + resolution_index + "]");
    that.attr("checked", true);
}
//Creat resolution list
function Creat_resolution_list() {
    for (var i = 0; i < resolution_array.length; i++) {
        var that=$("label[for=res_" + i + "]");
        that.text(resolution_array[i]);
        that.prev().val(resolution_list_value[i]);
    }
    get_mirascreen5g_flag();
   // $("#res_0_txt,#res_0,#res_2_txt,#res_2").remove();
   // set_resolutionradio();
}
//read resolution value
function Get_resolution() {

    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, { type: 'get_resolution'},function(resolution) {
		console.log("Get_resolution : "+resolution);
            var res = resolution.indexOf("15");
            if (res >= 0) {
                $('#res_value').val("15");
                resolution_index = 3;
                $('#resolution_sel').text("1024x768_60P"); //FMT_1024x768_60P
            } else {
                res = resolution.indexOf("4");
                if (res >= 0) {
                    $('#res_value').val("4");
                    resolution_index = 1;
                    $('#resolution_sel').text("1920x1080_24P");
                } else {
                    res = resolution.indexOf("2");
                    if (res >= 0) {
                        $('#res_value').val("2");
                        resolution_index = 2;
                        $('#resolution_sel').text("1920x1080_30P");
                    } else {
                        res = resolution.indexOf("17");
                        if (res >= 0) {
                            $('#res_value').val("17");
                            resolution_index = 5;
                            $('#resolution_sel').text("1920x1080_60P"); //FMT_1920x1080_60P
                        } else {
                            res = resolution.indexOf("5");
                            if (res >= 0) {
                                $('#res_value').val("5");
                                resolution_index = 0;
                                $('#resolution_sel').text("1280x720_60P");
                            }else{
                                res=resolution.indexOf("13");
                                if(res>=0){
                                    $('#res_value').val("13");
                                    resolution_index=4;
                                    $('#resolution_sel').text("720x480_60P"); //FMT_720x480_60P
                                }
                            }
                        }
                    }
                }
            }
        current_resolution_index = resolution_index;
        resolution_current_value_num = $('#res_value').val();
        resolution_current_value = $('#resolution_sel').text();
        Creat_resolution_list();

    },"text");
}



//default_mode button click
function default_mode_listener() {
    $('#defmode_ok').click(function(e) {
        $(document.body).css("overflow", "visible");
        if (save_defaultmode_index != defaultmode_index) {
            $('#Default_mode_value').val(defaultmode_index);
            $('#default_mode').text("" + defaultmode_array[defaultmode_index - 1]);

            $.post('cgi-bin/set_defaultmode_POST.cgi', $('#Default_mode_value').serialize());
            save_defaultmode_index = defaultmode_index;
            $("#defaultmode").hide();
        }
    });

    $('input:radio[name="de_item"]').click(function() {
        var that = $(this).val();
        var b = $(this).prev().text();
        defaultmode_index = that;
        $('#def-mode_text').text(that + b); //print user
        $('#Default_mode_value').val(that);

    });

    $("#defmode_cel").click(function() {
        defaultmode_index = save_defaultmode_index;
        $(document.body).css("overflow", "visible");
        $("#defaultmode").css("display","none");

    });


    $("#li_defaultmode").click(function() {
		console.log("---$(#li_defaultmode).click-----");
	$("#defaultmode").show();

        $('input:radio[name="de_item"]').each(function() {
            $(this).attr("checked", "");
        });
        set_defaultmode_radio();
        //$(document.body).css("overflow", "hidden");
	
    });

}
//set default_mode css
function set_defaultmode_radio() {
    var that = $("input[type=radio][name=de_item][value=" + defaultmode_index + "]");
    that.attr("checked", "checked");

}
//read default_mode val
function Get_defaultmode_UI() {

    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_last_ui'},function(last_ui_index) {
	//console.log("Get_defaultmode_UI: "+last_ui_index);
        if (last_ui_index.indexOf("1") >= 0) {
            defaultmode_index = 1;//EZCast Pro
            save_defaultmode_index = defaultmode_index;
        } else if (last_ui_index.indexOf("2") >= 0) {
            defaultmode_index = 2;//EZMirror+Timer
            save_defaultmode_index = defaultmode_index;
        }
        $('#default_mode').text("" + defaultmode_array[defaultmode_index - 1]);
        set_defaultmode_radio();
    },"text");
}

//read ezair value
function Get_ezairmode_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_ezairmode'},function(ret) {
        var value = Number(ret);
	console.log("Get_ezairmode_ui: "+value);
        ezairmode_index = value;
        ezairmode_select = ezairmode_index;
        set_ezairmode_style();
    }, "text");
}

//ezair button
function ezairmode_listener() {
    $("#ezairmode_ok").click(function(e) {
        set_ezairmode_ui();
        $("#ezairmode").hide();
    });

    $("#ezairmode_cel").click(function(e) {
        if (ezairmode_select != ezairmode_index) {
            ezairmode_select = ezairmode_index;
        }
        $("#ezairmode").css("display","none");
    });

    $('input:radio[name="ezairmodemode_item"]').click(function() {
        var that = $(this).val();
        ezairmode_select = that;
        select_txt = ezairmode_array[ezairmode_select];
    });

    $("#ezair").click(function() {
	$("#ezairmode").show();
        $('input:radio[name="ezairmodemode_item"]').each(function() {
            $(this).attr("checked", "");
        });
        set_ezairmode_style();
	
    });

}

//set ezari css
function set_ezairmode_style() {
    var that = $("input[type=radio][name=ezairmodemode_item][value=" + ezairmode_index + "]");
    that.attr("checked", "checked");
    select_txt = ezairmode_array[ezairmode_index];
    $("#ezairmode_txt").text(select_txt);
}
//set ezair value
function set_ezairmode_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var str = ezairmode_select + "\n" + "set_ezairmode\n";
    $.get(cgiurl, {type: str},function(ret) {
    }, "text");
    $("#ezairmode_txt").text(select_txt);
    ezairmode_index = ezairmode_select;
}

function reload_page() {
    window.location.href = "websetting.html";
}
//�޸�����
function change_psw() {
    $.post('cgi-bin/password_POST.cgi', $('#appsk').serialize());
    setTimeout(reload_page, 1000);
}
//����password
function set_appsk() {
    var errorFlag = 0;
    var regx = /^[A-Za-z0-9]+$/;
    appskValue = $('#appsk').val();
 if (appskValue != appskValue_save)
    {
    	if (!regx.test(appskValue))
	{
            $('#psk_warn_txt').text(PSW_ALLOW_string);
            errorFlag = 1;
       } 
	else 
	{
        	if (appskValue.length < 8 || appskValue.length > 15)
	 	{
	               $('#psk_warn_txt').text(PSW_LEN_string);
	               $('#app_sk').text("" + appskValue_save);
	               errorFlag = 1;
            	} 
	   	 else
		{
               	 errorFlag = 0;
               }
        }
        if (!errorFlag) 
	 {
            $('#app_sk').text(appskValue);
            $('#psk_warn_txt').text("Password modified successfully!");
            change_psw();
        }
    }
    else 
    {
     	$('#psk_warn_txt').text(LIST_WARN3_string);  //"Please enter a new password to save again");
    }
}
//�������ð�����Ӧ
function Set_pws_listener() {
    $("#set_appsk_ok").click(function() {
        set_appsk();
    });
	
    $("#appsk").keyup(function(){
        //alert(event.keyCode);
        if(event.keyCode==13){
            set_appsk();
        }
    });
    $("#pass_sel").click(function() {
        $('#appsk').val($('#app_sk').text());
        $('#Password').show();
    });
    $("#set_appsk_cel").click(function() {
        $('#Password').hide();
    });
}
//��ȡС������
function Get_password() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_softap_psk'},function(appsk) {
		console.log("Get_password"+appsk);
        $('#app_sk').text(appsk);
        $('#appsk').text(appsk);
        appskValue_save = $('#app_sk').text();
    },"text");
}
//read mac
function Get_AP_mac_ip() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: '1\nget_mac_ip'},function(mac_wlan1_ip) {
        if (mac_wlan1_ip.length) {
            start = 0;
            text_string_length = 0;
            text_string = mac_wlan1_ip;
            text_string_length = mac_wlan1_ip.length;
            $('#connect_mac_address').text(Get_text_from_textstring(text_string));
        }
    },"text");
}

//any button click
function click_listener() {

    clearInterval(delay_get_data_id);
    delay_get_data_id = -1;
    default_mode_listener();
    Set_pws_listener();
    Set_resolution_listener();
    Upgrade_listener();
    Upgrade_OK_listener();
    ezairmode_listener();
}

function delay_get_data_function() {
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    Get_defaultmode_UI();
    Get_resolution();
    Get_connected_ssid();
    Get_password();
    Get_local_version();
    Get_ezairmode_ui();
    Get_AP_mac_ip();

    delay_get_data_id = self.setInterval(click_listener, 1000);

}

//first run
function delay_get_data() {
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    delay_get_data_id = self.setInterval(delay_get_data_function, 100); 
}
