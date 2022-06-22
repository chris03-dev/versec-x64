if [ -s changelog.txt ] 
then
	git add .
	if [ $(git commit -F changelog.txt) ]
	then
		git push origin main
		git push mirror main
		git pull origin main
		rm changelog.txt
	fi
else
	echo "Please include a description of changes in changelog.txt before committing. Abort."
fi