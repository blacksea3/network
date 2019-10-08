//添加板块
//标题不能为空 标题备注限制50字符、描述限制100字符、字符任意。
//1个汉字为1个字符

$(function(){
    //表单验证
    $("#form-article-add").validate({
        //注意：这里这些形如blocktitle的标记是form里面的name,不是id!!!
        rules:{
            articletitle:{
                required:true,
                maxlength:50,
            },
            articlecontent:{
                required:true,
            },
            articleremark:{
                maxlength:50,
            },          
        },
        onkeyup:false,
        focusInvalid:true,
        success:"valid",
        submitHandler:function(form)
        {
            $.ajax({
                url: "../article-add/", async: true,           
                    data:
                    {
                        title: $("#articletitle").val(),
                        content: CKEDITOR.instances.articlecontent.getData(),
                        blockid: $("#articlecolumn option:selected").val(),
                        remark: $("#articleremark").val(),
                        attachids: $("#hiddeninfo").text(),
                        csrfmiddlewaretoken:$("#form-article-add").find("input[name='csrfmiddlewaretoken']").val()
                    }, success:
                    function (data) {
                        if(data == 'T')
                        {
                            alert('添加成功');
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

function imgupload()
{
    var formData  = new FormData();
    if ($('#articleimgfile').val() == "") {
        alert("empty?");
    }
    formData.append("upload", 1);
    formData.append("upfile", $("#articleimgfile")[0].files[0]);
    $.ajax({
        type: "POST",
        url: "../article-upload-file/",
        cache : false,
        //enctype: 'multipart/form-data',
        processData: false,
        contentType: false,
        data: formData,
        success: function (data) {
            var code = data.substring(0,1);
            var para = data.substring(1,data.length);
            if(code == 'T')
            {
                $('#hiddeninfo').text(para);
                alert("首页图上传成功!");
            }
            else
            {
                alert(para);
            }
        }
    });
};

function removeIframe()
{
    var index = parent.layer.getFrameIndex(window.name);  
    parent.layer.close(index); 
};
