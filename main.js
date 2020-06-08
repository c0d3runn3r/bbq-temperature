const http=require("http");
const config=require("./config.json");


(async ()=>{

	try {

		let data=await get(config.probe_url);
		console.log(data);

	} catch(e) {

		console.log(e);
	}

})();



function get(url="") {

	return new Promise((res, rej)=>{

		http.get(config.probe_url, (result)=>{

			let status_code=result.statusCode;
			let content_type=result.headers["content-type"];

			if(status_code != 200) {

				rej("Serve returned status "+status_code);
				return;
			}

			if(content_type != "application/json") {

				rej(`Server returned content-type'${content_type}'`);
				return;
			}

			result.setEncoding('utf8');
			let data="";
			result
				.on("data", (chunk)=>{ data+=chunk; })
				.on("end",()=>{

					try{

						res(JSON.parse(data));

					} catch(e) {

						rej(e);
					}

				 })
				.on("error",(e)=>{

					rej(e);
					return;
				});


		});
	});
}



