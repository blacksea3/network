//下拉菜单页面重定向

function article_list_change_by_bid()
{
	var bid = $("#blockselect option:selected").val();
	window.location.href = "../article-list.html/?bid=" + bid;
};
