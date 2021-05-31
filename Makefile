all : clean testaro

clean : clear_exec
	rm -f fich fich2 fich3 test.test

clear_exec :
	rm -f testaro boucle_infinie process_killer
	
testaro :
	gcc -Wall -Wformat -o testaro src/testaro.c
	
process_killer :
	gcc -Wall -Wformat -o process_killer tests/process_killer.c
	
test_marche : testaro
	./testaro tests/tst.testaro
	
test_erreur : testaro
	./testaro tests/tst_erreur.testaro
	
test_boucle : testaro
	./testaro tests/tst_boucle_inf.testaro
	
test_post : testaro
	./testaro tests/tst_post.testaro
	
test_erreur_cd : testaro
	./testaro tests/tst_erreur_cd.testaro