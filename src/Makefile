default:
	cd fwk && gmake
	cd activity && gmake
	cd rep && gmake
	cd engine && gmake
	cd main && gmake
	gmake runtest

clean:
	cd fwk && gmake clean
	cd activity && gmake clean
	cd rep && gmake clean
	cd engine && gmake clean
	cd main && gmake clean

runtest:
	cd engine && gmake runtest
	cd rep && gmake runtest
	cd main && gmake runtest
