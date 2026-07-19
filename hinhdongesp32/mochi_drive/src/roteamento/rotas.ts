import { Router } from 'express';
import Rotas from '../Controladores/ClassRequest';

const router = Router();

router.get("/", Rotas.methodGet)
router.get("/:id", Rotas.methodGet)

// Criar um novo registro
router.post("/", Rotas.methodPost);

// Atualizar um registro específico (Corrigido de methodPetch para methodPatch)
router.patch("/:id", Rotas.methodPatch);

// Deletar um registro específico
router.delete("/:id", Rotas.methodDelete);

export default router;