import { ObjectId } from "mongodb";
import { Request, Response } from "express";
import conectDataBase from "../conect_data/conectData";

class Rotas {
  // GET: Busca todos ou busca por ID
  static async methodGet(req: Request, res: Response) {
    try {
     const id = req.params.id as string;
     // Se houver ID na URL, busca um específico
     if (id) {
       // Valida se o formato do ObjectId é válido para evitar crash
        if (!ObjectId.isValid(id)) return res.status(400).json({ message: "ID em formato inválido." });

        const data = await conectDataBase.findOne({ _id: new ObjectId(id) });
        
        if (!data) return res.status(404).json({ message: "Registro não encontrado." });
        
        return res.status(200).json(data);
      } 
      
      // Se não houver ID, busca todos
      const data = await conectDataBase.find({}).toArray();
      return res.status(200).json(data);
      
    } catch (error) {
      return res.status(500).json({ mensagem: (error as Error).message });
    }
  }

  // POST: Cria um novo registro
  static async methodPost(req: Request, res: Response) {
    try {
      // Dica: Seria bom validar se o req.body não está vazio aqui
      await conectDataBase.insertOne(req.body);
      return res.status(201).json({ message: "Criado com sucesso" });
    } catch (error) {
      return res.status(500).json({ message: `${(error as Error).message} - Falha ao cadastrar` });
    }
  }

  // PATCH: Atualiza dados parciais
  static async methodPatch(req: Request, res: Response) {
    try {
      const id = req.params.id as string;

      if (!id || !ObjectId.isValid(id)) {
        return res.status(400).json({ message: "ID ausente ou inválido." });
      }

      const result = await conectDataBase.findOneAndUpdate(
        { _id: new ObjectId(id) }, 
        { $set: req.body },
        { returnDocument: 'after' } // Opcional: retorna o documento já atualizado se precisar
      );

      // No driver nativo do Mongo, você pode verificar se encontrou/modificou algo
      if (!result) {
        return res.status(404).json({ message: "Registro não encontrado para atualização." });
      }

      return res.status(200).json({ message: "Dados atualizados com sucesso!" }); 
    } catch (error) {
      return res.status(500).json({ message: `${(error as Error).message} - Falha na atualização` });
    }
  }

  // DELETE: Remove um registro
  static async methodDelete(req: Request, res: Response) {
    try {
      const id = req.params.id as string;

      if (!id || !ObjectId.isValid(id)) {
        return res.status(400).json({ message: "ID ausente ou inválido." });
      }

      const result = await conectDataBase.findOneAndDelete({ _id: new ObjectId(id) });

      if (!result) {
        return res.status(404).json({ message: "Registro não encontrado para exclusão." });
      }

      return res.status(200).json({ message: "Dados excluídos com sucesso" });
    } catch (error) {
      return res.status(500).json({ message: `${(error as Error).message} - Falha ao Deletar` });
    }
  }
}

export default Rotas;