class ConnectJson {
    static _myHeaders() {
        return new Headers({
            'Content-Type': 'application/json',
        });
    }
    
    static _optionsOBJ(cacheData){
        return {
            method: "POST",
            body: JSON.stringify(cacheData),
            headers: this._myHeaders(),
            mode: "cors",
            cache: "default",
        };
    }

    static async connectJsonUrlJson(URL, cacheData = null){
      
        let options = cacheData !== null ? this._optionsOBJ(cacheData) : null //POST <-> GET
       
        try{
            const conexao = await fetch(URL, options)
            if(conexao.status === 200){
                const openConexao = await conexao.json();
                return openConexao;
            }else{
                return conexao;
            }
        }catch(error){
            throw new Error("Erro no servidor");
        }
    }
}

// return await CadastrarRal.connectJsonUrlJson('http://clr0an001372366.nt.embratel.com.br:8003/host', bodyObj);