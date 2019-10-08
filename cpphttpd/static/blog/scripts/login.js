//登陆
$("#login").click(function(){
	$('#hiddeninfo').css("visibility","hidden");
	var check_code;										//检查代号值（含义为两位二进制）
	var username_ret = /^[a-zA-Z0-9_]{8,16}$/;
	var username = $("#username").val();
    if(username_ret.test(username))
	{
		//用户名格式正确，低位为0
        check_code = 0;
    }
	else
	{
		//用户名不正确，低位为0
        check_code = 1;
	}
	
	var password_ret = /^[a-zA-Z0-9_]{8,16}$/;
	var password = $("#password").val();
    if(password_ret.test(password))
	{
		//密码格式正确，高位为0
        check_code = check_code;
    }
	else
	{
		//密码格式不正确，高位为1
        check_code = check_code + 2;
	}	
	
	if(check_code == 0)
	{
		var online;
		if($("#online").prop('checked'))
			online = 1;
		else
			online = 0;
		$.ajax({
			url: "../login/", async: true,           
				data: {
					username:username,
					password:password,
					online:online,
					csrfmiddlewaretoken:$("#loginform").find("input[name='csrfmiddlewaretoken']").val()
			}, success:
				function (data) {
					if (data=='T')
					{
						//登陆成功，转向主页
						window.location = "../index";
					}
					else 
					{
						if(data[1] == '0')
						{
							$('#hiddeninfo').css("visibility","visible");
							$('#hiddeninfo').text("账号不存在");
						}
						else if(data[1] == '1')
						{
							$('#hiddeninfo').css("visibility","visible");
							$('#hiddeninfo').text("账号密码不对应");
						}
						else
						{
							$('#hiddeninfo').css("visibility","visible");
							$('#hiddeninfo').text("服务器内部错误");
						}
					}
				}, type: "POST"
		});
	}
	else if(check_code == 1)
	{
		$('#hiddeninfo').css("visibility","visible");
		$('#hiddeninfo').text("账号应由数字、字母、下划线组成，长度在8-16之间");
	}
	else if(check_code == 2)
	{
		$('#hiddeninfo').css("visibility","visible");
		$('#hiddeninfo').text("密码应由数字、字母、下划线组成，长度在8-16之间");
	}
	else
	{
		$('#hiddeninfo').css("visibility","visible");
		$('#hiddeninfo').text("账号和密码都应由数字、字母、下划线组成，长度在8-16之间");
	}	
})
