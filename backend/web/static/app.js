// ---------- state ----------
const state = {
  token: localStorage.getItem("ktester_token") || "",
  me:    null,
};

// ---------- helpers ----------
async function api(path, { method = "GET", body = null, raw = false } = {}) {
  const headers = {};
  if (state.token) headers["Authorization"] = "Bearer " + state.token;
  if (body && !raw) headers["Content-Type"] = "application/json";
  const res = await fetch(path, {
    method,
    headers,
    body: body == null ? undefined : (raw ? body : JSON.stringify(body)),
  });
  if (res.status === 401) {
    logout();
    throw new Error("登录已失效，请重新登录");
  }
  if (!res.ok) {
    let msg = res.statusText;
    try { const j = await res.json(); msg = j.error || msg; } catch {}
    throw new Error(msg);
  }
  if (raw) return res;
  if (res.status === 204) return null;
  const ct = res.headers.get("Content-Type") || "";
  return ct.includes("application/json") ? res.json() : res.text();
}

function toast(msg, isErr = false) {
  let t = document.querySelector(".toast");
  if (!t) {
    t = document.createElement("div"); t.className = "toast";
    document.body.appendChild(t);
  }
  t.textContent = msg;
  t.classList.toggle("err", isErr);
  t.classList.add("show");
  clearTimeout(t._h); t._h = setTimeout(() => t.classList.remove("show"), 2200);
}

function el(html) {
  const t = document.createElement("template");
  t.innerHTML = html.trim();
  return t.content.firstChild;
}

function logout() {
  state.token = ""; state.me = null;
  localStorage.removeItem("ktester_token");
  location.hash = "#/login";
  render();
}

function roleLabel(r) {
  if (r === "super_admin") return `<span class="role-pill super">超级管理员</span>`;
  if (r === "admin")       return `<span class="role-pill admin">管理员</span>`;
  return `<span class="role-pill user">用户</span>`;
}

function escapeHtml(s) {
  if (s == null) return "";
  return String(s)
    .replaceAll("&", "&amp;").replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;").replaceAll('"', "&quot;");
}

// ---------- routing ----------
function currentRoute() {
  const h = location.hash || "#/devices";
  return h.replace(/^#/, "");
}

window.addEventListener("hashchange", render);

async function render() {
  const view = document.getElementById("view");
  const nav  = document.getElementById("nav");
  const ua   = document.getElementById("userArea");
  view.innerHTML = ""; nav.innerHTML = ""; ua.innerHTML = "";

  // Need login?
  if (!state.token) {
    location.hash.startsWith("#/register") ? renderRegister(view) : renderLogin(view);
    return;
  }

  // Ensure me
  if (!state.me) {
    try { state.me = await api("/api/auth/me"); }
    catch { return logout(); }
  }

  // Top user area
  ua.innerHTML = `${escapeHtml(state.me.username)} ${roleLabel(state.me.role)} ` +
    `<button class="ghost" id="btnPw">修改密码</button>` +
    `<button class="ghost" id="btnLogout">退出</button>`;
  ua.querySelector("#btnLogout").onclick = logout;
  ua.querySelector("#btnPw").onclick = () => location.hash = "#/password";

  // Nav
  const tabs = [
    { id: "devices", label: "设备/配列",   roles: ["user","admin","super_admin"] },
    { id: "users",   label: "用户管理",   roles: ["admin","super_admin"] },
  ];
  const cur = currentRoute();
  for (const t of tabs) {
    if (!t.roles.includes(state.me.role)) continue;
    const b = document.createElement("button");
    b.textContent = t.label;
    if (cur.startsWith("/" + t.id)) b.classList.add("active");
    b.onclick = () => location.hash = "#/" + t.id;
    nav.appendChild(b);
  }

  // Route
  const r = currentRoute();
  if (r.startsWith("/devices")) return renderDevices(view);
  if (r.startsWith("/users"))   return renderUsers(view);
  if (r.startsWith("/password"))return renderPassword(view);
  location.hash = "#/devices";
}

// ---------- views ----------
function renderLogin(view) {
  const box = el(`
    <div class="auth-box">
      <h1>登录</h1>
      <div class="tabs">
        <button class="active" id="tabL">登录</button>
        <button id="tabR">注册</button>
      </div>
      <label>用户名</label><input id="u" autocomplete="username" />
      <label>密码</label><input id="p" type="password" autocomplete="current-password" />
      <div style="margin-top:14px"><button id="ok" style="width:100%">登录</button></div>
    </div>`);
  view.appendChild(box);
  box.querySelector("#tabR").onclick = () => location.hash = "#/register";
  box.querySelector("#ok").onclick = async () => {
    try {
      const j = await api("/api/auth/login", {
        method: "POST",
        body: { username: box.querySelector("#u").value, password: box.querySelector("#p").value },
      });
      state.token = j.token;
      localStorage.setItem("ktester_token", j.token);
      state.me = null;
      location.hash = "#/devices";
      render();
    } catch (e) { toast(e.message, true); }
  };
}

function renderRegister(view) {
  const box = el(`
    <div class="auth-box">
      <h1>注册</h1>
      <div class="tabs">
        <button id="tabL">登录</button>
        <button class="active" id="tabR">注册</button>
      </div>
      <label>用户名 (3-32)</label><input id="u" />
      <label>密码 (≥6)</label><input id="p" type="password" />
      <div style="margin-top:14px"><button id="ok" style="width:100%">注册并登录</button></div>
      <p class="muted" style="text-align:center; margin-top:10px">
        新账号默认为 <b>普通用户</b>，仅可查看配列。
      </p>
    </div>`);
  view.appendChild(box);
  box.querySelector("#tabL").onclick = () => location.hash = "#/login";
  box.querySelector("#ok").onclick = async () => {
    try {
      const j = await api("/api/auth/register", {
        method: "POST",
        body: { username: box.querySelector("#u").value, password: box.querySelector("#p").value },
      });
      state.token = j.token;
      localStorage.setItem("ktester_token", j.token);
      state.me = null;
      location.hash = "#/devices";
      render();
    } catch (e) { toast(e.message, true); }
  };
}

function renderPassword(view) {
  const box = el(`
    <div class="card" style="max-width:480px; margin:0 auto">
      <h2>修改密码</h2>
      <label>当前密码</label><input id="o" type="password" />
      <label>新密码 (≥6)</label><input id="n" type="password" />
      <div style="margin-top:14px"><button id="ok">提交</button></div>
    </div>`);
  view.appendChild(box);
  box.querySelector("#ok").onclick = async () => {
    try {
      await api("/api/auth/change-password", {
        method: "POST",
        body: {
          old_password: box.querySelector("#o").value,
          new_password: box.querySelector("#n").value,
        },
      });
      toast("已修改");
      location.hash = "#/devices";
    } catch (e) { toast(e.message, true); }
  };
}

// ---------- devices ----------
async function renderDevices(view) {
  const canEdit = state.me.role === "admin" || state.me.role === "super_admin";
  view.innerHTML = `<div class="card"><h2>设备 / 配列</h2>
    <div id="list">加载中...</div>
    ${canEdit ? `<div style="margin-top:14px"><button id="add">+ 新增设备</button></div>` : ""}
  </div>`;
  if (canEdit) view.querySelector("#add").onclick = () => editDeviceDialog(null);
  try {
    const list = await api("/api/devices");
    const tbl = `
      <table>
        <thead><tr>
          <th>VID-PID</th><th>名称</th><th>说明</th>
          <th>配列文件</th><th>更新时间</th>
          ${canEdit ? `<th>操作</th>` : ""}
        </tr></thead>
        <tbody>
          ${(list||[]).map(d => `
            <tr>
              <td><code>${escapeHtml(d.vid)}-${escapeHtml(d.pid)}</code></td>
              <td>${escapeHtml(d.name)}</td>
              <td>${escapeHtml(d.description || "")}</td>
              <td><a href="/api/devices/${d.vid}-${d.pid}/layout?token=${encodeURIComponent(state.token)}">${escapeHtml(d.layout_file)}</a></td>
              <td class="muted">${d.updated_at?.slice(0,19).replace("T"," ") || ""}</td>
              ${canEdit ? `<td class="actions">
                <button class="ghost" data-edit="${d.id}">编辑</button>
                <button class="danger" data-del="${d.id}" data-name="${escapeHtml(d.vid+'-'+d.pid)}">删除</button>
              </td>` : ""}
            </tr>`).join("")}
        </tbody>
      </table>`;
    view.querySelector("#list").innerHTML = list && list.length ? tbl : "<p class='muted'>暂无设备</p>";
    if (canEdit) {
      view.querySelectorAll("[data-edit]").forEach(b => b.onclick = () => {
        const d = list.find(x => x.id == b.dataset.edit); editDeviceDialog(d);
      });
      view.querySelectorAll("[data-del]").forEach(b => b.onclick = async () => {
        if (!confirm(`确定删除 ${b.dataset.name}？此操作不可恢复。`)) return;
        try { await api("/api/devices/" + b.dataset.del, { method: "DELETE" }); toast("已删除"); renderDevices(view); }
        catch (e) { toast(e.message, true); }
      });
    }
  } catch (e) {
    view.querySelector("#list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

function editDeviceDialog(existing) {
  const isNew = !existing;
  const view = document.getElementById("view");
  view.innerHTML = `
    <div class="card">
      <h2>${isNew ? "新增设备" : "编辑设备 #" + existing.id}</h2>
      <div class="row">
        <div><label>VID (0xHHHH)</label>
          <input id="vid" value="${existing ? escapeHtml(existing.vid) : "0x"}" ${existing ? "readonly" : ""}/>
        </div>
        <div><label>PID (0xHHHH)</label>
          <input id="pid" value="${existing ? escapeHtml(existing.pid) : "0x"}" ${existing ? "readonly" : ""}/>
        </div>
      </div>
      <label>设备名称</label><input id="name" value="${existing ? escapeHtml(existing.name) : ""}"/>
      <label>说明</label><input id="desc" value="${existing ? escapeHtml(existing.description||"") : ""}"/>
      <label>配列文件名 (如 wp75.c / xp75.json，可含子目录)</label>
      <input id="file" value="${existing ? escapeHtml(existing.layout_file) : ""}"/>
      <label>配列内容 (留空则不修改文件，仅改元数据)</label>
      <textarea id="body" placeholder="粘贴 .c 或 .json 内容"></textarea>
      <div style="margin-top:14px" class="actions">
        <button id="ok">${isNew ? "创建" : "保存"}</button>
        <button class="ghost" id="cancel">取消</button>
      </div>
    </div>`;
  view.querySelector("#cancel").onclick = () => { location.hash = "#/devices"; };
  view.querySelector("#ok").onclick = async () => {
    const payload = {
      name: view.querySelector("#name").value,
      description: view.querySelector("#desc").value,
      layout_file: view.querySelector("#file").value,
      layout_body: view.querySelector("#body").value,
    };
    try {
      if (isNew) {
        payload.vid = view.querySelector("#vid").value;
        payload.pid = view.querySelector("#pid").value;
        if (!payload.layout_body) {
          toast("新建时必须粘贴配列内容", true); return;
        }
        await api("/api/devices", { method: "POST", body: payload });
      } else {
        await api("/api/devices/" + existing.id, { method: "PUT", body: payload });
      }
      toast("已保存");
      location.hash = "#/devices";
    } catch (e) { toast(e.message, true); }
  };
}

// ---------- users ----------
async function renderUsers(view) {
  const isSuper = state.me.role === "super_admin";
  view.innerHTML = `<div class="card">
    <h2>用户管理</h2>
    <div id="list">加载中...</div>
    ${isSuper ? `<div style="margin-top:14px"><button id="add">+ 新增用户</button></div>` : ""}
  </div>`;
  if (isSuper) view.querySelector("#add").onclick = () => editUserDialog(null);
  try {
    const list = await api("/api/users");
    const tbl = `
      <table>
        <thead><tr>
          <th>ID</th><th>用户名</th><th>角色</th>
          <th>创建</th><th>最近登录</th>
          ${isSuper ? `<th>操作</th>` : ""}
        </tr></thead>
        <tbody>
          ${(list||[]).map(u => `
            <tr>
              <td>${u.id}</td>
              <td><b>${escapeHtml(u.username)}</b></td>
              <td>${roleLabel(u.role)}</td>
              <td class="muted">${(u.created_at||"").slice(0,19).replace("T"," ")}</td>
              <td class="muted">${(u.last_login||"").slice(0,19).replace("T"," ")}</td>
              ${isSuper ? `<td class="actions">
                <button class="ghost" data-edit="${u.id}">编辑</button>
                ${u.id !== state.me.id ? `<button class="danger" data-del="${u.id}" data-name="${escapeHtml(u.username)}">删除</button>` : ""}
              </td>` : ""}
            </tr>`).join("")}
        </tbody>
      </table>
      ${!isSuper ? `<p class="muted" style="margin-top:10px">（只有超级管理员可以新增/修改用户。）</p>` : ""}
    `;
    view.querySelector("#list").innerHTML = tbl;
    if (isSuper) {
      view.querySelectorAll("[data-edit]").forEach(b => b.onclick = () => {
        const u = list.find(x => x.id == b.dataset.edit); editUserDialog(u);
      });
      view.querySelectorAll("[data-del]").forEach(b => b.onclick = async () => {
        if (!confirm(`确定删除用户 ${b.dataset.name}？`)) return;
        try { await api("/api/users/" + b.dataset.del, { method: "DELETE" }); toast("已删除"); renderUsers(view); }
        catch (e) { toast(e.message, true); }
      });
    }
  } catch (e) {
    view.querySelector("#list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

function editUserDialog(existing) {
  const isNew = !existing;
  const view = document.getElementById("view");
  view.innerHTML = `
    <div class="card" style="max-width:520px;margin:0 auto">
      <h2>${isNew ? "新增用户" : "编辑用户 #" + existing.id}</h2>
      <label>用户名</label>
      <input id="u" value="${existing ? escapeHtml(existing.username) : ""}" ${existing ? "readonly" : ""}/>
      <label>角色</label>
      <select id="r">
        <option value="user">普通用户</option>
        <option value="admin">管理员</option>
        <option value="super_admin">超级管理员</option>
      </select>
      <label>${isNew ? "密码 (≥6)" : "新密码 (留空则不改)"}</label>
      <input id="p" type="password"/>
      <div style="margin-top:14px" class="actions">
        <button id="ok">${isNew ? "创建" : "保存"}</button>
        <button class="ghost" id="cancel">取消</button>
      </div>
    </div>`;
  if (existing) view.querySelector("#r").value = existing.role;
  view.querySelector("#cancel").onclick = () => location.hash = "#/users";
  view.querySelector("#ok").onclick = async () => {
    try {
      if (isNew) {
        await api("/api/users", {
          method: "POST",
          body: {
            username: view.querySelector("#u").value,
            password: view.querySelector("#p").value,
            role:     view.querySelector("#r").value,
          },
        });
      } else {
        const body = { role: view.querySelector("#r").value };
        const pw = view.querySelector("#p").value;
        if (pw) body.password = pw;
        await api("/api/users/" + existing.id, { method: "PUT", body });
      }
      toast("已保存");
      location.hash = "#/users";
    } catch (e) { toast(e.message, true); }
  };
}

// ---------- boot ----------
render();
