	use32

	imul	bl
	imul	byte ptr [esi]
	imul	bx
	imul	word ptr [esi]
	imul	ebx
	imul	dword ptr [esi]

	imul	ax,bx
	imul	ax,[esi]
	imul	eax,ebx
	imul	eax,[esi]

	imul	ax,bx,22
	imul	ax,[esi],22
	imul	eax,ebx,22
	imul	eax,[esi],22

	imul	ax,[22]
	imul	eax,[22]
	imul	ax,#22
	imul	eax,#22

	imul	ax,bx,300
	imul	ax,[esi],300
	imul	eax,ebx,300000
	imul	eax,[esi],300000

	imul	ax,[300]
	imul	eax,[300000]
	imul	ax,#300
	imul	eax,#300000
