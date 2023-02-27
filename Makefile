default:
	cd os && make clean
	cd os && make
	cd apps/lottery_test && make clean
	cd apps/lottery_test && make
	cd apps/lottery_test && make run

q2:
	cd os && make clean
	cd os && make
	cd apps/example && make clean
	cd apps/example && make
	cd apps/example && make run