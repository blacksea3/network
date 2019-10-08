//编辑板块
//标题不能为空 标题备注限制50字符、描述限制100字符、字符任意。
//1个汉字为1个字符

$(function(){
	$('.skin-minimal input').iCheck({
		checkboxClass: 'icheckbox-blue',
		radioClass: 'iradio-blue',
		increaseArea: '20%'
	});
	
	//表单验证
	$("#form-block-edit").validate({
		//注意：这里这些形如blocktitle的标记是form里面的name,不是id!!!
		rules:{
			blocktitle:{
				required:true,
				maxlength:50,
			},
			blockdescription:{
				maxlength:100,
			},
			blockremark:{
				maxlength:50,
			},			
		},
		onkeyup:false,
		focusInvalid:true,
		success:"valid",
		submitHandler:function(form)
		{
			$.ajax({
				url: "../block-edit/", async: true,           
					data: 
					{
						bid: $("#blockid").text(),
						title: $("#blocktitle").val(),
						description: $("#blockdescription").val(),
						remark: $("#blockremark").val(),
						csrfmiddlewaretoken:$("#form-block-edit").find("input[name='csrfmiddlewaretoken']").val()
					}, success:
					function (data) {
						if(data == 'T')
						{
							alert('修改成功');
							removeIframe();
						}
						else
						{
							alert('服务器内部错误！');
						}
					}, type: "POST"
			});
		}
	});

	state = "pending";
});

function removeIframe()
{
	var index = parent.layer.getFrameIndex(window.name);  
	parent.layer.close(index); 
};
