define disp
	layout src
	set print addr off
	break main
	run < input > output
	display c
	display i
end
